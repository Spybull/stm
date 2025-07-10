#include "ssh.h"
#include "utils.h"
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <libssh/callbacks.h>

static struct termios terminal;
ssh_channel chan;
int signal_delayed = 0;

static void sigwindowchanged(int i){
  (void) i;
  signal_delayed=1;
}


static int
ssh_auth_callback_l(const char *prompt, char *buf, size_t len,
                       int echo, int verify, void *userdata stm_unused)
{
    return ssh_getpass(prompt, buf, len, echo, verify);
}

static struct ssh_callbacks_struct cb = {
		.userdata = NULL
};


static void
sigterm_exit(stm_unused int i)
{
    tcsetattr(0, TCSANOW, &terminal);
    exit(0);
}

static int
verify_knownhost(ssh_session session)
{
    ssh_key server_pubkey;
    unsigned char *server_pubkey_hash = NULL;
    size_t server_pubkey_hash_size;
    int state;
    int rc;

    state = ssh_session_is_known_server(session);

    rc = ssh_get_server_publickey(session, &server_pubkey);
    if (SSH_OK != rc) return -1;

    rc = ssh_get_publickey_hash(server_pubkey,
                                SSH_PUBLICKEY_HASH_SHA256,
                                &server_pubkey_hash,
                                &server_pubkey_hash_size);

    if (SSH_OK != rc) return -1;
    ssh_key_free(server_pubkey);

    switch (state)
    {
        case SSH_SERVER_KNOWN_OK:
            /* nothing to do */
        break;

        case SSH_SERVER_KNOWN_CHANGED:
            fprintf(stderr,"Host key for server changed : server's one is now :\n");
            ssh_print_hash(SSH_PUBLICKEY_HASH_SHA256, server_pubkey_hash, server_pubkey_hash_size);
            ssh_clean_pubkey_hash(&server_pubkey_hash);
            fprintf(stderr,"For security reason, connection will be stopped\n");
        return -1;
        
        case SSH_SERVER_FOUND_OTHER:
            fprintf(stderr,"The host key for this server was not found but an other type of key exists.\n");
        return -1;

        case SSH_SERVER_FILE_NOT_FOUND:
            // check in database
        case SSH_SERVER_NOT_KNOWN:
            fprintf(stderr, "Could not find known host file. If you accept the host key here\n");
            fprintf(stderr, "The server is unknown. Do you trust the host key ?\n");
        break;

        case SSH_SERVER_ERROR:
            ssh_clean_pubkey_hash(&server_pubkey_hash);
            fprintf(stderr,"%s",ssh_get_error(session));
            return -1;
    }

    ssh_clean_pubkey_hash(&server_pubkey_hash);
    return 0;
}

static int 
ssh_console_auth(ssh_session session, const char *password)
{
    int method, rc;

    rc = ssh_userauth_none(session, NULL);
    if (rc == SSH_AUTH_ERROR)
        return rc;

    method = ssh_userauth_list(session, NULL);
    int retries = 5;

    while (retries--)
    {
        if ( (method & SSH_AUTH_METHOD_PASSWORD) && password && *password ){
            rc = ssh_userauth_password(session, NULL, password);
        } else if (method & SSH_AUTH_METHOD_PUBLICKEY) {
            rc = ssh_userauth_publickey_auto(session, NULL, NULL);
            
        } else {
            fprintf(stderr, "No supported authentication method available\n");
            return SSH_AUTH_DENIED;
        }

        if (rc == SSH_AUTH_SUCCESS)
            break;

        if (rc == SSH_AUTH_ERROR || rc == SSH_AUTH_DENIED)
            return rc;

    }

    return rc;
}
static void setsignal(void){
    signal(SIGWINCH, sigwindowchanged);
    signal_delayed=0;
}

static void sizechanged(void){
    struct winsize win = { 0, 0, 0, 0 };
    ioctl(1, TIOCGWINSZ, &win);
    ssh_channel_change_pty_size(chan,win.ws_col, win.ws_row);
    setsignal();
}

static void
select_loop(ssh_session session,ssh_channel channel)
{
    ssh_connector connector_in, connector_out, connector_err;
    ssh_event event = ssh_event_new();

    /* stdin */
    connector_in = ssh_connector_new(session);
    ssh_connector_set_out_channel(connector_in, channel, SSH_CONNECTOR_STDOUT);
    ssh_connector_set_in_fd(connector_in, 0);
    ssh_event_add_connector(event, connector_in);

    /* stdout */
    connector_out = ssh_connector_new(session);
    ssh_connector_set_out_fd(connector_out, 1);
    ssh_connector_set_in_channel(connector_out, channel, SSH_CONNECTOR_STDOUT);
    ssh_event_add_connector(event, connector_out);

    /* stderr */
    connector_err = ssh_connector_new(session);
    ssh_connector_set_out_fd(connector_err, 2);
    ssh_connector_set_in_channel(connector_err, channel, SSH_CONNECTOR_STDERR);
    ssh_event_add_connector(event, connector_err);

    while(ssh_channel_is_open(channel)){
        if(signal_delayed)
            sizechanged();
        ssh_event_dopoll(event, 60000);
    }
    ssh_event_remove_connector(event, connector_in);
    ssh_event_remove_connector(event, connector_out);
    ssh_event_remove_connector(event, connector_err);

    ssh_connector_free(connector_in);
    ssh_connector_free(connector_out);
    ssh_connector_free(connector_err);

    ssh_event_free(event);
    ssh_channel_free(channel);
}


static int
shell(ssh_session session)
{
    ssh_channel channel;
    struct termios terminal_local;
    int interactive = isatty(0);

    channel = ssh_channel_new(session);

    if ( interactive ) {
        tcgetattr(0, &terminal_local);
        memcpy(&terminal, &terminal_local, sizeof(struct termios));
    }

    if(ssh_channel_open_session(channel)){
        printf("error opening channel : %s\n",ssh_get_error(session));
        return -1;
    }
    chan=channel;
    if(interactive){
        ssh_channel_request_pty(channel);
        sizechanged();
    }

    if(ssh_channel_request_shell(channel)){
        printf("Requesting shell : %s\n",ssh_get_error(session));
        return -1;
    }

    if(interactive) {
        cfmakeraw(&terminal_local);
        tcsetattr(0, TCSANOW, &terminal_local);
        setsignal();
    }

    signal(SIGTERM, sigterm_exit);
    select_loop(session,channel);
    tcsetattr(0, TCSANOW, &terminal);
    
    if(interactive)
        return 0;

    return 0;
}


static int
new_ssh_session(ssh_session session, libstm_server *srv, libstm_error_t *err)
{
    int rc = 0;

    ssh_options_set(session, SSH_OPTIONS_USER, srv->login);
    ssh_options_set(session, SSH_OPTIONS_HOST, srv->ip);
    ssh_options_set(session, SSH_OPTIONS_PORT, &srv->port);
    ssh_options_parse_config(session, NULL); // use ~/.ssh/config

    rc = ssh_connect(session);
    if (SSH_OK != rc)
        return stm_make_error(err, errno, "Connection failed: %s", ssh_get_error(session));

    rc = verify_knownhost(session);
    if (rc != 0)
        return stm_make_error(err, 0, "failed to verify knownhost error");

    /* This authentication method is used to tell the client
       which methods are accepted by the server  */
    ssh_userauth_none(session, NULL);

    int auth = ssh_console_auth(session, srv->creds);
    if (auth != SSH_AUTH_SUCCESS)
        return stm_make_error(err, 0, "%s", ssh_get_error(session));

    return shell(session);

}

int
libstm_ssh_connect(libstm_server *srv, libstm_error_t *err)
{
    ssh_session session;
    session = ssh_new();
    if (stm_unlikely(session == NULL))
        stm_oom();

    cb.auth_function = ssh_auth_callback_l;
    cb.userdata = NULL;

    ssh_callbacks_init(&cb);
    ssh_set_callbacks(session, &cb);
    signal(SIGTERM, sigterm_exit);
    return new_ssh_session(session, srv, err);
}



ssh_user_data_t *
libstm_parse_user_host(const char *user_host, libstm_error_t *err) {

    char *puh = xstrdup(user_host);
    char *user = NULL, *host = NULL;

    char *p = strrchr(puh, '@');
    if ( p == NULL ) { // if @ does not exists this is the host
        host = puh;
    } else {
        *p++ = '\0';
        user = xstrdup(puh);
        if ( strlen(p) == 0 )
            host = xstrdup("localhost");
        else
            host = xstrdup(p);
    }
    
    if (user == NULL || *user == '\0') {
        user = whoami(err);
        if (stm_unlikely(user == NULL))
            return NULL;
    }

    ssh_user_data_t *user_data = xmalloc0(sizeof(ssh_user_data_t));
    user_data->login = user;
    user_data->host = host;
    
    return user_data;
}

ssh_session
libstm_ssh_connect_once(const char *host, const char *user, const char *password, libstm_error_t *err)
{
    ssh_session session = ssh_new();
    if (session == NULL) {
        stm_make_error(err, errno, "failed to initialize ssh session");
        return NULL;
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, host);
    ssh_options_set(session, SSH_OPTIONS_USER, user);
    /*int verbosity = SSH_LOG_PROTOCOL;
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);*/

    if (ssh_connect(session) != SSH_OK) {
        stm_make_error(err, 0, "Error connecting to host: %s", ssh_get_error(session));
        ssh_free(session);
        return NULL;
    }
    
    if (ssh_userauth_none(session, NULL) == SSH_AUTH_ERROR) {
        stm_make_error(err, 0, "ssh_userauth_none failed: %s", ssh_get_error(session));
        ssh_disconnect(session);
        ssh_free(session);
        return NULL;
    }

    if (password && ssh_userauth_password(session, NULL, password) == SSH_AUTH_SUCCESS)
        return session;

    stm_make_error(err, 0, "Authentication failed: %s\n", ssh_get_error(session));
    ssh_disconnect(session);
    ssh_free(session);
    return NULL;
}

char *
libstm_ssh_exec_cmd(ssh_session *session, const char *cmd)
{
    ssh_channel channel = ssh_channel_new(*session);
    if (!channel)
        return NULL;

    if (ssh_channel_open_session(channel) != SSH_OK)
        goto cleanup;

    if (ssh_channel_request_exec(channel, cmd) != SSH_OK)
        goto cleanup;

    char buffer[256];
    size_t total = 0;
    char *output = malloc(4096);
    if (!output)
        goto cleanup;

    output[0] = '\0';

    int n;
    while ((n = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        if (total + n >= 4096)
            break; // упрощённая защита
        strcat(output, buffer);
        total += n;
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return output;

cleanup:
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return NULL;

     return 0;
}
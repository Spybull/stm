#ifndef QUERIES_H
#define QUERIES_H

const char SELECT_ALL_SERVERS[] =
    "SELECT * FROM SERVERS;";

const char ADD_SERVER[] =
    "INSERT INTO SERVERS (name, ip, port, proto, login, creds, description) VALUES "
    "(:name, :ip, :port, :proto, :login, :creds, :description);";

// INSERT INTO SERVERS_META (name) VALUES (:name);
const char ADD_SERVER_META[] =
    "INSERT INTO SERVERS_META (name) VALUES (:name);";

// DELETE FROM SERVERS WHERE name = ?;
const char DELETE_SERVER[] = 
    "DELETE FROM SERVERS WHERE name = ?;";

// DELETE FROM SERVER_META WHERE name = ?;
const char DELETE_SERVER_META[] =
    "DELETE FROM SERVER_META WHERE name = ?;";

const char SELECT_ALL_WHERE_NAME_XXX[] =
    "SELECT * FROM SERVERS WHERE name = '%s';";

#endif
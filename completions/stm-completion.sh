_stm()
{
    local cur prev words cword
    _init_completion -n = || return

    local cmds="init creds server"
    local server_subcmds="add del list ssh"

    local server_add_opts="--creds= --description= --interactive --ip= --protocol= --login= --port="
    local server_add_short_opts="-c -d -i -k -l -p"

    local server_list_opts="--format= --no-headers"
    local server_list_short_opts="-f -n"

    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    words=("${COMP_WORDS[@]}")
    cword=$COMP_CWORD

    if [[ $cword -eq 1 ]]; then
        COMPREPLY=( $(compgen -W "$cmds" -- "$cur") )
        return 0
    fi

    case "${words[1]}" in
        server)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=( $(compgen -W "$server_subcmds" -- "$cur") )
                return 0
            fi

            case "${words[2]}" in
                add)
                    COMPREPLY=( $(compgen -W "$server_add_opts $server_add_short_opts" -- "$cur") )
                    return 0
                    ;;
                list)
                    COMPREPLY=( $(compgen -W "$server_list_opts $server_list_short_opts" -- "$cur") )
                    return 0
                    ;;
                ssh|del)
                    return 0
                    ;;
            esac
            ;;
        creds)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=( $(compgen -W "store status kill" -- "$cur") )
                return 0
            fi
            ;;
    esac
}
complete -F _stm stm

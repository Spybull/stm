COMP_WORDBREAKS=${COMP_WORDBREAKS//=}

_stm() {
    local cur prev words cword
    
    if [[ "${COMP_LINE:COMP_POINT-1:1}" == "=" ]]; then
        ((COMP_POINT--))
    fi

    local db="$HOME/.stm/stm_meta.db"
    local names=""
    if [[ -f "$db" ]]; then
        names=$(sqlite3 "$db" "SELECT name FROM SERVERS_META;" 2>/dev/null)
    fi

    _init_completion -n = || return

    local cmds="init creds server"
    local server_subcmds="add del list ssh find"

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
                find)
                    if [[ "$cur" == --name* ]]; then

                        if [[ "$cur" == "--name" ]]; then
                            COMPREPLY=( "--name=" )
                            compopt -o nospace
                            return 0
                        fi

                        if [[ "$cur" == --name=* ]]; then
                            local prefix="--name="
                            local partial="${cur#--name=}"
                            local completions=()
                            for n in $names; do
                                [[ "$n" == "$partial"* ]] && completions+=("${prefix}${n}")
                            done
                            COMPREPLY=( "${completions[@]}" )
                            compopt -o nospace
                            return 0
                        fi
                    fi

                    if [[ "$prev" == "--name" || "$prev" == "-n" ]]; then
                        COMPREPLY=( $(compgen -W "$names" -- "$cur") )
                        return 0
                    fi

                    if [[ "$cur" == --* ]]; then
                        COMPREPLY=( $(compgen -W "--name= --ip=" -- "$cur") )
                        compopt -o nospace
                        return 0
                    fi

                    return 0
                    ;;
                add)
                    COMPREPLY=( $(compgen -W "$server_add_opts $server_add_short_opts" -- "$cur") )
                    return 0
                    ;;
                list)
                    COMPREPLY=( $(compgen -W "$server_list_opts $server_list_short_opts" -- "$cur") )
                    return 0
                    ;;
                ssh|del)
                    COMPREPLY=( $(compgen -W "$names" -- "$cur") )
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

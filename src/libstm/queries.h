#ifndef QUERIES_H
#define QUERIES_H

const char ADD_SERVER[] =
    "INSERT INTO SERVERS (name, ip, port, password, description) VALUES "
    "(:name, :ip, :port, :password, :description);";

const char SELECT_ALL_WHERE_NAME_XXX[] =
    "SELECT * FROM SERVERS WHERE name = '%s';";

#endif
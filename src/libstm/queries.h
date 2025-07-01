#ifndef QUERIES_H
#define QUERIES_H

const char ADD_SERVER[] =
    "INSERT INTO SERVERS (name, ip, port, password, description) VALUES "
    "(:name, :ip, :port, :password, :description);";

#endif
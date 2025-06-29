#ifndef QUERIES_H
#define QUERIES_H

const char ADD_SERVER[] =
    "INSERT INTO SERVERS (name, ip, port, description) VALUES "
    "(:name, :ip, :port, :description);";

#endif
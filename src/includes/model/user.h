#ifndef USER_H
#define USER_H

typedef struct User
{
    int id;
    char *email;
    char *password;
    char *firstname;
    char *lastname;
    char *telnumber;
    char *username;
} User;

#endif
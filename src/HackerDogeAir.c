#include <kore/kore.h>
#include <kore/http.h>

int     loginpost(struct http_request *);

int 
loginpost(struct http_request *req)
{
    char *username;
    char *password;
    struct kore_buf *buf;

    buf = kore_buf_alloc(128); //TODO: check buff overflow @ append

    http_populate_post(req);
    if(http_argument_get_string(req, "username", &username))
    {
        kore_buf_appendf(buf, "username: '%s' \n", username);
    }
    if(http_argument_get_string(req, "password", &password))
    {
        kore_buf_appendf(buf, "password: '%s' \n", password);
    }
    kore_log(LOG_NOTICE, "username: '%s' password: '%s' ", username, password);

    http_response_header(req, "content-type", "text/plain");
    http_response(req, 200, buf->data, buf->offset);
    kore_buf_free(buf);

    return (KORE_RESULT_OK);
}
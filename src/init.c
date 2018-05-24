#include <kore/kore.h>
#include <kore/pgsql.h>


int    init(int);

int
init(int state)
{
    /* Register our database. */
    kore_pgsql_register("db", "host=localhost dbname=HackerDogeAir user=hackerdoge password=root");

    return (KORE_RESULT_OK);
}
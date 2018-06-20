#include <kore/kore.h>
#include <kore/pgsql.h>


int    init(int);

int
init(int state)
{
    /* Register our database. */
    /* This will result as a string in our binary and is not save at all!! */
    kore_pgsql_register("DogeAir", "host=localhost dbname=DogeAir user=hackerdoge password=doge");

    return (KORE_RESULT_OK);
}
#include <stdio.h>
#include "parcel.h"

int main(void)
{
    // small sample json
    const char* json = "{ \"test\":\"json\" }";
    
    const char* str = parcel_getItemFromJSON(json, "test");
    puts(str);
}

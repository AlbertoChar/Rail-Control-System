#include "../include/includeF.h"


// Maps
// {start position, end position, path}
const railMaps maps[N_MAPS] = {
        {{ "S1", "S6", "MA1-MA2-MA3-MA8" },
                { "S2", "S6", "MA5-MA6-MA7-MA3-MA8" },
                { "S7", "S3", "MA13-MA12-MA11-MA10-MA9" },
                { "S4", "S8", "MA14-MA15-MA16-MA12" },
                { "", "", "" }},
        {{ "S2", "S6", "MA5-MA6-MA7-MA3-MA8" },
                { "S3", "S8", "MA9-MA10-MA11-MA12" },
                { "S4", "S8", "MA14-MA15-MA16-MA12" },
                { "S6", "S1", "MA8-MA3-MA2-MA1" },
                { "S5", "S1", "MA4-MA3-MA2-MA1" }}};


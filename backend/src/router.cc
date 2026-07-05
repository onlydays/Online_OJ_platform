#include "router.h"
#include "handlers/auth_handler.h"
#include "handlers/user_handler.h"
#include "handlers/problem_handler.h"
#include "handlers/submission_handler.h"

#include <httplib.h>

void registerAllRoutes(httplib::Server& server) {
    registerAuthRoutes(server);
    registerUserRoutes(server);
    registerProblemRoutes(server);
    registerSubmissionRoutes(server);
}

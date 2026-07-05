#ifndef PROBLEM_HANDLER_H
#define PROBLEM_HANDLER_H

namespace httplib { class Server; }

void registerProblemRoutes(httplib::Server& server);

#endif

#ifndef AUTH_HANDLER_H
#define AUTH_HANDLER_H

namespace httplib { class Server; }

void registerAuthRoutes(httplib::Server& server);

#endif // AUTH_HANDLER_H

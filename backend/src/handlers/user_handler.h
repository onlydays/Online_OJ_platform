#ifndef USER_HANDLER_H
#define USER_HANDLER_H

namespace httplib { class Server; }

void registerUserRoutes(httplib::Server& server);

#endif

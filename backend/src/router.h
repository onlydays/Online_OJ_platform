#ifndef ROUTER_H
#define ROUTER_H

namespace httplib { class Server; }

void registerAllRoutes(httplib::Server& server);

#endif

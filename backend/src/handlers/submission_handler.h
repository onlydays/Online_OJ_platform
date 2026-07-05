#ifndef SUBMISSION_HANDLER_H
#define SUBMISSION_HANDLER_H

namespace httplib { class Server; }

void registerSubmissionRoutes(httplib::Server& server);

#endif

#ifndef _TVW_SESSION_MANAGER
#define _TVW_SESSION_MANAGER


#include <sysinc.hpp>
#include <grutil.hpp>
#include <grthread.hpp>


class Session;

class SessionManager
{
public:
    SessionManager();
    ~SessionManager();

    void Add(Session *session);
    void Remove(const guid_t *sid);

    Session* LookupSession(const guid_t* sid);
private:
    std::map<guid_t, Session*> m_sessions;
    GrRecursiveMutex m_session_mutex;
};

#endif

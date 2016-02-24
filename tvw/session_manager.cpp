#include "session_manager.h"
#include "session.h"
#include "logger.h"


SessionManager::SessionManager()
{
}


SessionManager::~SessionManager()
{
}

void SessionManager::Add(Session *session)
{
    if (session == NULL)
        return;

    const guid_t *sid = session->GetId();
    Session *s = LookupSession(session->GetId());
    if (s){
        return;
    }
    GrAutoLockT<GrRecursiveMutex> guard(m_session_mutex);
    m_sessions.insert(std::make_pair(*sid, session));
}

void SessionManager::Remove(const guid_t *sid)
{
    if (sid == NULL)
        return;

    GrAutoLockT<GrRecursiveMutex> guard(m_session_mutex);
    m_sessions.erase(*sid);
}

Session* SessionManager::LookupSession(const guid_t* sid)
{
    GrAutoLockT<GrRecursiveMutex> guard(m_session_mutex);
    std::map<guid_t, Session*>::iterator it;

    it = m_sessions.find(*sid);
    if (it == m_sessions.end())
    {
        return NULL;
    }
    return it->second;
}

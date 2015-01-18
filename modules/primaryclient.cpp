/*
 * Copyright (C) 2004-2012  See the AUTHORS file for details.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <znc/Chan.h>
#include <znc/User.h>
#include <znc/IRCNetwork.h>
#include <znc/Modules.h>
#include <znc/Query.h>

using std::vector;

class CPrimaryClient : public CModule {
public:
	MODCONSTRUCTOR(CPrimaryClient) {}

	void ClearAllBuffers() {
		CIRCNetwork* pNetwork = GetNetwork();
		if (pNetwork) {
			const vector<CChan*>& vChans = pNetwork->GetChans();

			for (vector<CChan*>::const_iterator it = vChans.begin(); it != vChans.end(); ++it) {
				// Skip detached channels, they weren't read yet
				if ((*it)->IsDetached())
					continue;

				(*it)->ClearBuffer();
			}

			vector<CQuery*> VQueries = pNetwork->GetQueries();

			for (vector<CQuery*>::const_iterator it = VQueries.begin(); it != VQueries.end(); ++it) {
				pNetwork->DelQuery((*it)->GetName());
			}

			pNetwork->ClearNoticeBuffer();
		}
	}

	void OnClientLoginEarly() {
		// if the new client has the same id as a client already connected
		// consider it a double connection due to ping timeout or other connection
		// problems that might have kept the first instance of the client hanging.
		if (!m_pClient->GetIdentifier().empty() && m_pNetwork) {
			vector<CClient*> vClients = m_pNetwork->GetClients();

			for (unsigned int a = 0; a < vClients.size(); a++) {
				if (vClients[a] != m_pClient) {
					if (vClients[a]->GetIdentifier().Equals(m_pClient->GetIdentifier())) {
						ClearAllBuffers();
					}
				}
			}
		}
	}

	void OnClientLogin() {
		// after a client connects that has an identifier and was sent
		// all the buffers we clear them out
		if (!m_pClient->GetIdentifier().empty()) {
			ClearAllBuffers();
		}
	}


	void OnClientDisconnect() {
		if (!m_pClient->GetIdentifier().empty() && m_pNetwork) {
			ClearAllBuffers();
		}
	}

};

NETWORKMODULEDEFS(CPrimaryClient, "Clears buffers when a client with identifier disconnects.")

/*
 * This file is part of IPAACA, the
 *  "Incremental Processing Architecture
 *   for Artificial Conversational Agents".  
 *
 * Copyright (c) 2009-2015 Social Cognitive Systems Group
 *                         (formerly the Sociable Agents Group)
 *                         CITEC, Bielefeld University
 *
 * http://opensource.cit-ec.de/projects/ipaaca/
 * http://purl.org/net/ipaaca
 *
 * This file may be licensed under the terms of of the
 * GNU Lesser General Public License Version 3 (the ``LGPL''),
 * or (at your option) any later version.
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the LGPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the LGPL along with this
 * program. If not, go to http://www.gnu.org/licenses/lgpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  
 *
 * The development of this software was supported by the
 * Excellence Cluster EXC 277 Cognitive Interaction Technology.
 * The Excellence Cluster EXC 277 is a grant of the Deutsche
 * Forschungsgemeinschaft (DFG) in the context of the German
 * Excellence Initiative.
 */

/**
 * \file   util/notifier.cc
 *
 * \brief Component notification (i.e. module-level introspection).
 *
 * \author Ramin Yaghoubzadeh (ryaghoubzadeh@uni-bielefeld.de)
 * \date   March, 2015
 */

#include <ipaaca/util/notifier.h>

namespace ipaaca {
namespace util {

ComponentNotifier::~ComponentNotifier() 
{
	//LOG_IPAACA_CONSOLE("~ComponentNotifier")
	if (initialized) {
		go_down();
	}
}

ComponentNotifier::ComponentNotifier(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& recvCategories)
: initialized(false), gone_down(false), name(componentName), function(componentFunction)
{
	send_categories = ipaaca::str_join(sendCategories, ",");
	recv_categories = ipaaca::str_join(recvCategories, ",");
	// create private in/out buffer pair since none was specified
	out_buf = ipaaca::OutputBuffer::create(componentName);
	in_buf = ipaaca::InputBuffer::create(componentName, _IPAACA_COMP_NOTIF_CATEGORY);
}

ComponentNotifier::ComponentNotifier(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& receiveCategories, ipaaca::OutputBuffer::ptr outBuf, ipaaca::InputBuffer::ptr inBuf)
: initialized(false), name(componentName), function(componentFunction), out_buf(outBuf), in_buf(inBuf)
{
	send_categories = ipaaca::str_join(sendCategories, ",");
	recv_categories = ipaaca::str_join(receiveCategories, ",");
}

ComponentNotifier::ptr ComponentNotifier::create(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& recvCategories)
{
	return ComponentNotifier::ptr(new ComponentNotifier(componentName, componentFunction, sendCategories, recvCategories));
}
ComponentNotifier::ptr ComponentNotifier::create(const std::string& componentName, const std::string& componentFunction, const std::set<std::string>& sendCategories, const std::set<std::string>& recvCategories, ipaaca::OutputBuffer::ptr outBuf, ipaaca::InputBuffer::ptr inBuf)
{
	return ComponentNotifier::ptr(new ComponentNotifier(componentName, componentFunction, sendCategories, recvCategories, outBuf, inBuf));
}

void ComponentNotifier::handle_iu_event(IUInterface::ptr iu, IUEventType event_type, bool local)
{
	if ((event_type == IU_ADDED) || (event_type == IU_UPDATED) || (event_type == IU_MESSAGE)) {
		Locker locker(lock);
		IPAACA_DEBUG("Received a componentNotify")
		std::string cName = iu->payload()[_IPAACA_COMP_NOTIF_NAME];
		std::string cState = iu->payload()[_IPAACA_COMP_NOTIF_STATE];
		if (cName != name) {
			// call all registered notification handlers
			for (std::vector<IUEventHandlerFunction>::iterator it = _handlers.begin(); it != _handlers.end(); ++it) {
				(*it)(iu, event_type, local);
			}
			// send own info only if the remote component is a newly initialized one
			if (cState=="new") {
				//IPAACA_DEBUG("Submitting own componentNotify for new remote component")
				submit_notify(_IPAACA_COMP_NOTIF_STATE_OLD);
			}
		}
	}
}

void ComponentNotifier::add_notification_handler(ipaaca::IUEventHandlerFunction function)
{
	Locker locker(lock);
	_handlers.push_back(function);
}

void ComponentNotifier::submit_notify(const std::string& current_state)
{
	ipaaca::Message::ptr iu = ipaaca::Message::create(_IPAACA_COMP_NOTIF_CATEGORY);
	iu->payload()[_IPAACA_COMP_NOTIF_NAME] = name;
	iu->payload()[_IPAACA_COMP_NOTIF_STATE] = current_state;
	iu->payload()[_IPAACA_COMP_NOTIF_FUNCTION] = function;
	iu->payload()[_IPAACA_COMP_NOTIF_SEND_CATS] = send_categories;
	iu->payload()[_IPAACA_COMP_NOTIF_RECV_CATS] = recv_categories;
	out_buf->add(iu);
	IPAACA_DEBUG( "Sending a componentNotify: " << name << ": " << current_state << " (" << function << ", " << send_categories << ", " << recv_categories << ")" )
}

void ComponentNotifier::initialize() {
	Locker locker(lock);
	if (!initialized) {
		initialized = true;
		in_buf->register_handler(boost::bind(&ComponentNotifier::handle_iu_event, this, _1, _2, _3));
		submit_notify(_IPAACA_COMP_NOTIF_STATE_NEW);
	}
}

void ComponentNotifier::go_down() {
	Locker locker(lock);
	if (initialized && (!gone_down)) {
		gone_down = true;
		submit_notify(_IPAACA_COMP_NOTIF_STATE_DOWN);
	}
}

}}


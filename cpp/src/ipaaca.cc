#include <ipaaca.h>
#include <cstdlib>

namespace ipaaca {

// util and init//{{{
std::string generate_uuid_string()
{
	uuid_t uuidt;
	uuid_string_t uuidstr;
	uuid_generate(uuidt);
	uuid_unparse_lower(uuidt, uuidstr);
	return uuidstr;
}

//const LinkSet EMPTY_LINK_SET = LinkSet();
//const std::set<std::string> EMPTY_LINK_SET();
       
void initialize_ipaaca_rsb()
{
	ParticipantConfig config = ParticipantConfig::fromConfiguration();
	Factory::getInstance().setDefaultParticipantConfig(config);
	
	boost::shared_ptr<IUConverter> iu_converter(new IUConverter());
	boost::shared_ptr<IUPayloadUpdateConverter> payload_update_converter(new IUPayloadUpdateConverter());
	boost::shared_ptr<IULinkUpdateConverter> link_update_converter(new IULinkUpdateConverter());
	stringConverterRepository()->registerConverter(iu_converter);
	stringConverterRepository()->registerConverter(payload_update_converter);
	stringConverterRepository()->registerConverter(link_update_converter);
	
	//IPAACA_TODO("initialize all converters")
}
/*
void init_inprocess_too() {
	//ParticipantConfig config = Factory::getInstance().getDefaultParticipantConfig();
	ParticipantConfig config = ParticipantConfig::fromFile("rsb.cfg");
	//ParticipantConfig::Transport inprocess = config.getTransport("inprocess");
	//inprocess.setEnabled(true);
	//config.addTransport(inprocess);
	Factory::getInstance().setDefaultParticipantConfig(config);
}
*/
//}}}

std::ostream& operator<<(std::ostream& os, const IUPayloadUpdate& obj)//{{{
{
	os << "PayloadUpdate(uid=" << obj.uid << ", revision=" << obj.revision;
	os << ", writer_name=" << obj.writer_name << ", is_delta=" << (obj.is_delta?"True":"False");
	os << ", new_items = {";
	bool first = true;
	for (std::map<std::string, std::string>::const_iterator it=obj.new_items.begin(); it!=obj.new_items.end(); ++it) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << it->first << "':'" << it->second << "'";
	}
	os << "}, keys_to_remove = [";
	first = true;
	for (std::vector<std::string>::const_iterator it=obj.keys_to_remove.begin(); it!=obj.keys_to_remove.end(); ++it) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << *it << "'";
	}
	os << "])";
	return os;
}
//}}}
std::ostream& operator<<(std::ostream& os, const IULinkUpdate& obj)//{{{
{
	os << "LinkUpdate(uid=" << obj.uid << ", revision=" << obj.revision;
	os << ", writer_name=" << obj.writer_name << ", is_delta=" << (obj.is_delta?"True":"False");
	os << ", new_links = {";
	bool first = true;
	for (std::map<std::string, std::set<std::string> >::const_iterator it=obj.new_links.begin(); it!=obj.new_links.end(); ++it) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << it->first << "': [";
		bool ffirst = true;
		for (std::set<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			if (ffirst) { ffirst=false; } else { os << ", "; }
			os << "'" << *it2 << "'";
		}
		os << "]";
	}
	os << "}, links_to_remove = {";
	first = true;
	for (std::map<std::string, std::set<std::string> >::const_iterator it=obj.links_to_remove.begin(); it!=obj.links_to_remove.end(); ++it) {
		if (first) { first=false; } else { os << ", "; }
		os << "'" << it->first << "': [";
		bool ffirst = true;
		for (std::set<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			if (ffirst) { ffirst=false; } else { os << ", "; }
			os << "'" << *it2 << "'";
		}
		os << "]";
	}
	os << "})";
	return os;
}
//}}}

// SmartLinkMap//{{{
void SmartLinkMap::_add_and_remove_links(const LinkMap& add, const LinkMap& remove)
{
	// remove specified links
	for (LinkMap::const_iterator it = remove.begin(); it != remove.end(); ++it ) {
		// if link type exists
		if (_links.count(it->first) > 0) {
			// remove one by one
			for (LinkSet::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
				_links[it->first].erase(*it2);
			}
			// wipe the type key if no more links are left
			if (_links[it->first].size() == 0) {
				_links.erase(it->first);
			}
		}
	}
	// add specified links
	for (LinkMap::const_iterator it = add.begin(); it != add.end(); ++it ) {
		for (LinkSet::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			_links[it->first].insert(*it2);
		}
	}
}
void SmartLinkMap::_replace_links(const LinkMap& links)
{
	//_links.clear();
	_links=links;
}
//}}}

// Buffer//{{{
void Buffer::_allocate_unique_name(const std::string& basename) {
	std::string uuid = ipaaca::generate_uuid_string();
	std::string name = basename + "-" + uuid.substr(0,8);
	_unique_name = name;
}
//}}}

// OutputBuffer//{{{

OutputBuffer::OutputBuffer(const std::string& basename)
:Buffer(basename)
{
}
void OutputBuffer::_send_iu_link_update(IUInterface* iu, bool is_delta, revision_t revision, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
{
	IPAACA_IMPLEMENT_ME
}
void OutputBuffer::_send_iu_payload_update(IUInterface* iu, bool is_delta, revision_t revision, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
	IPAACA_IMPLEMENT_ME
}
void OutputBuffer::_send_iu_commission(IUInterface* iu, revision_t revision, const std::string& writer_name)
{
	IPAACA_IMPLEMENT_ME
}
void OutputBuffer::add(IU::ref iu)
{
	//IPAACA_IMPLEMENT_ME
	if (_iu_store.count(iu->uid()) > 0) {
		throw IUPublishedError();
	}
	_iu_store[iu->uid()] = iu;
	iu->_associate_with_buffer(this); //shared_from_this());
	_publish_iu(iu);
}

void OutputBuffer::_publish_iu(IU::ref iu)
{
	Informer<AnyType>::Ptr informer = _get_informer(iu->_category);
	Informer<ipaaca::IU>::DataPtr iu_data(iu);
	informer->publish(iu_data);
}

Informer<AnyType>::Ptr OutputBuffer::_get_informer(const std::string& category)
{
	if (_informer_store.count(category) > 0) {
		return _informer_store[category];
	} else {
		IPAACA_INFO("making new informer for category " << category)
		std::string scope_string = "/ipaaca/category/" + category;
		Informer<AnyType>::Ptr informer = Factory::getInstance().createInformer<AnyType> ( Scope(scope_string));
		_informer_store[category] = informer;
		return informer;
	}
}
boost::shared_ptr<IU> OutputBuffer::remove(const std::string& iu_uid)
{
	IPAACA_IMPLEMENT_ME
}
boost::shared_ptr<IU> OutputBuffer::remove(IU::ref iu)
{
	IPAACA_IMPLEMENT_ME
}

/*
	def _send_iu_link_update(self, iu, is_delta, revision, new_links=None, links_to_remove=None, writer_name="undef"):
		'''Send an IU link update.
		
		Keyword arguments:
		iu -- the IU being updated
		is_delta -- whether this is an incremental update or a replacement
			the whole link dictionary
		revision -- the new revision number
		new_links -- a dictionary of new link sets
		links_to_remove -- a dict of the link sets that shall be removed
		writer_name -- name of the Buffer that initiated this update, necessary
			to enable remote components to filter out updates that originate d
			from their own operations
		'''
		if new_links is None:
			new_links = {}
		if links_to_remove is None:
			links_to_remove = {}
		link_update = IULinkUpdate(iu._uid, is_delta=is_delta, revision=revision)
		link_update.new_links = new_links
		if is_delta:
			link_update.links_to_remove = links_to_remove
		link_update.writer_name = writer_name
		informer = self._get_informer(iu._category)
		informer.publishData(link_update)
		# FIXME send the notification to the target, if the target is not the writer_name
*/
/*
	def _send_iu_payload_update(self, iu, is_delta, revision, new_items=None, keys_to_remove=None, writer_name="undef"):
		'''Send an IU payload update.
		
		Keyword arguments:
		iu -- the IU being updated
		is_delta -- whether this is an incremental update or a replacement
		revision -- the new revision number
		new_items -- a dictionary of new payload items
		keys_to_remove -- a list of the keys that shall be removed from the
		 	payload
		writer_name -- name of the Buffer that initiated this update, necessary
			to enable remote components to filter out updates that originate d
			from their own operations
		'''
		if new_items is None:
			new_items = {}
		if keys_to_remove is None:
			keys_to_remove = []
		payload_update = IUPayloadUpdate(iu._uid, is_delta=is_delta, revision=revision)
		payload_update.new_items = new_items
		if is_delta:
			payload_update.keys_to_remove = keys_to_remove
		payload_update.writer_name = writer_name
		informer = self._get_informer(iu._category)
		informer.publishData(payload_update)
*/
//}}}

// InputBuffer//{{{
InputBuffer::InputBuffer(const std::string& basename)
:Buffer(basename)
{
}
//}}}



// IUInterface//{{{

IUInterface::IUInterface()
: _buffer(NULL), _committed(false)
{
}

void IUInterface::_set_uid(const std::string& uid) {
	if (_uid != "") {
		throw IUAlreadyHasAnUIDError();
	}
	_uid = uid;
}

void IUInterface::_set_buffer(Buffer* buffer) { //boost::shared_ptr<Buffer> buffer) {
	if (_buffer) {
		throw IUAlreadyInABufferError();
	}
	_buffer = buffer;
	
}

void IUInterface::_set_owner_name(const std::string& owner_name) {
	if (_owner_name != "") {
		throw IUAlreadyHasAnOwnerNameError();
	}
	_owner_name = owner_name;
}

/// set the buffer pointer and the owner names of IU and Payload
void IUInterface::_associate_with_buffer(Buffer* buffer) { //boost::shared_ptr<Buffer> buffer) {
	_set_buffer(buffer); // will throw if already set
	_set_owner_name(buffer->unique_name());
	payload()._set_owner_name(buffer->unique_name());
}

void IUInterface::add_links(const std::string& type, const LinkSet& targets, const std::string& writer_name)
{
	LinkMap none;
	LinkMap add;
	add[type] = targets;
	_modify_links(true, add, none, writer_name);
	_add_and_remove_links(add, none);
}

void IUInterface::remove_links(const std::string& type, const LinkSet& targets, const std::string& writer_name)
{
	LinkMap none;
	LinkMap remove;
	remove[type] = targets;
	_modify_links(true, none, remove, writer_name);
	_add_and_remove_links(none, remove);
}

void IUInterface::modify_links(const LinkMap& add, const LinkMap& remove, const std::string& writer_name)
{
	_modify_links(true, add, remove, writer_name);
	_add_and_remove_links(add, remove);
}

void IUInterface::set_links(const LinkMap& links, const std::string& writer_name)
{
	LinkMap none;
	_modify_links(false, links, none, writer_name);
	_replace_links(links);
}

//}}}

// IU//{{{
IU::ref IU::create(const std::string& category, IUAccessMode access_mode, bool read_only, const std::string& payload_type)
{
	IU::ref iu = IU::ref(new IU(category, access_mode, read_only, payload_type)); /* params */ //));
	iu->_payload.initialize(iu);
	return iu;
}

IU::IU(const std::string& category, IUAccessMode access_mode, bool read_only, const std::string& payload_type)
{
	_revision = 1;
	_uid = ipaaca::generate_uuid_string();
	_category = category;
	_payload_type = payload_type;
	// payload initialization deferred to IU::create(), above
	_read_only = read_only;
	_access_mode = access_mode;
	_committed = false;
}

void IU::_modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
{
	_revision_lock.lock();
	if (_committed) {
		_revision_lock.unlock();
		throw IUCommittedError();
	}
	_increase_revision_number();
	if (is_published()) {
		_buffer->_send_iu_link_update(this, is_delta, _revision, new_links, links_to_remove, writer_name);
	}
	_revision_lock.unlock();
}
void IU::_modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
	_revision_lock.lock();
	if (_committed) {
		_revision_lock.unlock();
		throw IUCommittedError();
	}
	_increase_revision_number();
	if (is_published()) {
		_buffer->_send_iu_payload_update(this, is_delta, _revision, new_items, keys_to_remove, writer_name);
	}
	_revision_lock.unlock();
}

void IU::commit()
{
	_internal_commit();
}

void IU::_internal_commit(const std::string& writer_name)
{
	_revision_lock.lock();
	if (_committed) {
		_revision_lock.unlock();
		throw IUCommittedError();
	}
	_increase_revision_number();
	_committed = true;
	if (is_published()) {
		_buffer->_send_iu_commission(this, _revision, writer_name);
	}
	_revision_lock.unlock();
}
//}}}

// RemotePushIU//{{{

RemotePushIU::ref RemotePushIU::create()
{
	RemotePushIU::ref iu = RemotePushIU::ref(new RemotePushIU(/* params */));
	iu->_payload.initialize(iu);
	return iu;
}
RemotePushIU::RemotePushIU()
{
	// nothing
}
void RemotePushIU::_modify_links(bool is_delta, const LinkMap& new_links, const LinkMap& links_to_remove, const std::string& writer_name)
{
	IPAACA_IMPLEMENT_ME
}
void RemotePushIU::_modify_payload(bool is_delta, const std::map<std::string, std::string>& new_items, const std::vector<std::string>& keys_to_remove, const std::string& writer_name)
{
	IPAACA_IMPLEMENT_ME
}

void RemotePushIU::commit()
{
	IPAACA_IMPLEMENT_ME
}

//}}}




// PayloadEntryProxy//{{{

PayloadEntryProxy::PayloadEntryProxy(Payload* payload, const std::string& key)
: _payload(payload), _key(key)
{
}
PayloadEntryProxy& PayloadEntryProxy::operator=(const std::string& value)
{
	_payload->set(_key, value);
	return *this;
}
PayloadEntryProxy::operator std::string()
{
	return _payload->get(_key);
}
PayloadEntryProxy::operator long()
{
	return atol(operator std::string().c_str());
}
PayloadEntryProxy::operator double()
{
	return atof(operator std::string().c_str());
}
//}}}

// Payload//{{{

void Payload::initialize(boost::shared_ptr<IUInterface> iu)
{
	_iu = iu;
}

PayloadEntryProxy Payload::operator[](const std::string& key)
{
	//boost::shared_ptr<PayloadEntryProxy> p(new PayloadEntryProxy(this, key));
	return PayloadEntryProxy(this, key);
}

inline void Payload::set(const std::string& k, const std::string& v) {
	std::map<std::string, std::string> _new;
	std::vector<std::string> _remove;
	_new[k]=v;
	_iu->_modify_payload(true, _new, _remove, "" );
	_store[k] = v;
}
inline void Payload::remove(const std::string& k) {
	std::map<std::string, std::string> _new;
	std::vector<std::string> _remove;
	_remove.push_back(k);
	_iu->_modify_payload(true, _new, _remove, "" );
	_store.erase(k);
}
inline std::string Payload::get(const std::string& k) {
	if (_store.count(k)>0) return _store[k];
	else return IPAACA_PAYLOAD_DEFAULT_STRING_VALUE;
}
//}}}

// IUConverter//{{{

IUConverter::IUConverter()
: Converter<std::string> ("ipaaca::IU", "ipaaca-iu", true)
{
}

std::string IUConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	IPAACA_INFO("entering")
	// Ensure that DATA actually holds a datum of the data-type we expect.
	assert(data.first == getDataType()); // "ipaaca::IU"
	// NOTE: a dynamic_pointer_cast cannot be used from void*
	boost::shared_ptr<const IU> obj = boost::static_pointer_cast<const IU> (data.second);
	boost::shared_ptr<protobuf::IU> pbo(new protobuf::IU());
	// transfer obj data to pbo
	pbo->set_uid(obj->uid());
	pbo->set_revision(obj->revision());
	pbo->set_category(obj->category());
	pbo->set_payload_type(obj->payload_type());
	pbo->set_owner_name(obj->owner_name());
	pbo->set_committed(obj->committed());
	pbo->set_access_mode(ipaaca::protobuf::IU::PUSH); // TODO
	pbo->set_read_only(obj->read_only());
	for (std::map<std::string, std::string>::const_iterator it=obj->_payload._store.begin(); it!=obj->_payload._store.end(); ++it) {
		protobuf::PayloadItem* item = pbo->add_payload();
		item->set_key(it->first);
		item->set_value(it->second);
		item->set_type("str"); // FIXME other types than str (later)
	}
	for (LinkMap::const_iterator it=obj->_links._links.begin(); it!=obj->_links._links.end(); ++it) {
		protobuf::LinkSet* links = pbo->add_links();
		links->set_type(it->first);
		for (std::set<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			links->add_targets(*it2);
		}
	}
	pbo->SerializeToString(&wire);
	IPAACA_INFO("leaving")
	return getWireSchema();

}

AnnotatedData IUConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
	assert(wireSchema == getWireSchema()); // "ipaaca-iu"
	boost::shared_ptr<protobuf::IU> pbo(new protobuf::IU());
	pbo->ParseFromString(wire);
	IUAccessMode mode = static_cast<IUAccessMode>(pbo->access_mode());
	switch(mode) {
		case IU_ACCESS_PUSH:
			{
			// Create a "remote push IU"
			boost::shared_ptr<RemotePushIU> obj(new RemotePushIU());
			// transfer pbo data to obj
			obj->_uid = pbo->uid();
			obj->_revision = pbo->revision();
			obj->_category = pbo->category();
			obj->_payload_type = pbo->payload_type();
			obj->_owner_name = pbo->owner_name();
			obj->_committed = pbo->committed();
			obj->_read_only = pbo->read_only();
			obj->_access_mode = IU_ACCESS_PUSH;
			for (int i=0; i<pbo->payload_size(); i++) {
				const protobuf::PayloadItem& it = pbo->payload(i);
				obj->_payload._store[it.key()] = it.value();
			}
			for (int i=0; i<pbo->links_size(); i++) {
				const protobuf::LinkSet& pls = pbo->links(i);
				LinkSet& ls = obj->_links._links[pls.type()];
				for (int j=0; j<pls.targets_size(); j++) {
					ls.insert(pls.targets(j));
				}
			}
			return std::make_pair(getDataType(), obj);
			break;
			}
		default:
			// other cases not handled yet! ( TODO )
			throw NotImplementedError();
	}
}

//}}}

// IUPayloadUpdateConverter//{{{

IUPayloadUpdateConverter::IUPayloadUpdateConverter()
: Converter<std::string> ("ipaaca::IUPayloadUpdate", "ipaaca-iu-payload-update", true)
{
}

std::string IUPayloadUpdateConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	assert(data.first == getDataType()); // "ipaaca::IUPayloadUpdate"
	boost::shared_ptr<const IUPayloadUpdate> obj = boost::static_pointer_cast<const IUPayloadUpdate> (data.second);
	boost::shared_ptr<protobuf::IUPayloadUpdate> pbo(new protobuf::IUPayloadUpdate());
	// transfer obj data to pbo
	pbo->set_uid(obj->uid);
	pbo->set_revision(obj->revision);
	pbo->set_writer_name(obj->writer_name);
	pbo->set_is_delta(obj->is_delta);
	for (std::map<std::string, std::string>::const_iterator it=obj->new_items.begin(); it!=obj->new_items.end(); ++it) {
		protobuf::PayloadItem* item = pbo->add_new_items();
		item->set_key(it->first);
		item->set_value(it->second);
		item->set_type("str"); // FIXME other types than str (later)
	}
	for (std::vector<std::string>::const_iterator it=obj->keys_to_remove.begin(); it!=obj->keys_to_remove.end(); ++it) {
		pbo->add_keys_to_remove(*it);
	}
	pbo->SerializeToString(&wire);
	return getWireSchema();

}

AnnotatedData IUPayloadUpdateConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
	assert(wireSchema == getWireSchema()); // "ipaaca-iu-payload-update"
	boost::shared_ptr<protobuf::IUPayloadUpdate> pbo(new protobuf::IUPayloadUpdate());
	pbo->ParseFromString(wire);
	boost::shared_ptr<IUPayloadUpdate> obj(new IUPayloadUpdate());
	// transfer pbo data to obj
	obj->uid = pbo->uid();
	obj->revision = pbo->revision();
	obj->writer_name = pbo->writer_name();
	obj->is_delta = pbo->is_delta();
	for (int i=0; i<pbo->new_items_size(); i++) {
		const protobuf::PayloadItem& it = pbo->new_items(i);
		obj->new_items[it.key()] = it.value();
	}
	for (int i=0; i<pbo->keys_to_remove_size(); i++) {
		obj->keys_to_remove.push_back(pbo->keys_to_remove(i));
	}
	return std::make_pair(getDataType(), obj);
}

//}}}

// IULinkUpdateConverter//{{{

IULinkUpdateConverter::IULinkUpdateConverter()
: Converter<std::string> ("ipaaca::IULinkUpdate", "ipaaca-iu-link-update", true)
{
}

std::string IULinkUpdateConverter::serialize(const AnnotatedData& data, std::string& wire)
{
	assert(data.first == getDataType()); // "ipaaca::IULinkUpdate"
	boost::shared_ptr<const IULinkUpdate> obj = boost::static_pointer_cast<const IULinkUpdate> (data.second);
	boost::shared_ptr<protobuf::IULinkUpdate> pbo(new protobuf::IULinkUpdate());
	// transfer obj data to pbo
	pbo->set_uid(obj->uid);
	pbo->set_revision(obj->revision);
	pbo->set_writer_name(obj->writer_name);
	pbo->set_is_delta(obj->is_delta);
	for (std::map<std::string, std::set<std::string> >::const_iterator it=obj->new_links.begin(); it!=obj->new_links.end(); ++it) {
		protobuf::LinkSet* links = pbo->add_new_links();
		links->set_type(it->first);
		for (std::set<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			links->add_targets(*it2);
		}
	}
	for (std::map<std::string, std::set<std::string> >::const_iterator it=obj->links_to_remove.begin(); it!=obj->links_to_remove.end(); ++it) {
		protobuf::LinkSet* links = pbo->add_links_to_remove();
		links->set_type(it->first);
		for (std::set<std::string>::const_iterator it2=it->second.begin(); it2!=it->second.end(); ++it2) {
			links->add_targets(*it2);
		}
	}
	pbo->SerializeToString(&wire);
	return getWireSchema();

}

AnnotatedData IULinkUpdateConverter::deserialize(const std::string& wireSchema, const std::string& wire) {
	assert(wireSchema == getWireSchema()); // "ipaaca-iu-link-update"
	boost::shared_ptr<protobuf::IULinkUpdate> pbo(new protobuf::IULinkUpdate());
	pbo->ParseFromString(wire);
	boost::shared_ptr<IULinkUpdate> obj(new IULinkUpdate());
	// transfer pbo data to obj
	obj->uid = pbo->uid();
	obj->revision = pbo->revision();
	obj->writer_name = pbo->writer_name();
	obj->is_delta = pbo->is_delta();
	for (int i=0; i<pbo->new_links_size(); ++i) {
		const protobuf::LinkSet& it = pbo->new_links(i);
		for (int j=0; j<it.targets_size(); ++j) {
			obj->new_links[it.type()].insert(it.targets(j)); // = vec;
		}
	}
	for (int i=0; i<pbo->links_to_remove_size(); ++i) {
		const protobuf::LinkSet& it = pbo->links_to_remove(i);
		for (int j=0; j<it.targets_size(); ++j) {
			obj->links_to_remove[it.type()].insert(it.targets(j));
		}
	}
	return std::make_pair(getDataType(), obj);
}

//}}}


} // of namespace ipaaca



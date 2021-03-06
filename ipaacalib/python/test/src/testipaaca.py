# This file is part of IPAACA, the
#  "Incremental Processing Architecture
#   for Artificial Conversational Agents".	
#
# Copyright (c) 2009-2013 Sociable Agents Group
#                         CITEC, Bielefeld University	
#
# http://opensource.cit-ec.de/projects/ipaaca/
# http://purl.org/net/ipaaca
#
# This file may be licensed under the terms of of the
# GNU Lesser General Public License Version 3 (the ``LGPL''),
# or (at your option) any later version.
#
# Software distributed under the License is distributed
# on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
# express or implied. See the LGPL for the specific language
# governing rights and limitations.
#
# You should have received a copy of the LGPL along with this
# program. If not, go to http://www.gnu.org/licenses/lgpl.html
# or write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.	
#
# The development of this software was supported by the
# Excellence Cluster EXC 277 Cognitive Interaction Technology.
# The Excellence Cluster EXC 277 is a grant of the Deutsche
# Forschungsgemeinschaft (DFG) in the context of the German
# Excellence Initiative.

import sys
import time
import unittest

import hamcrest as hc
import ipaaca

def handle_iu_event(iu, event_type, local):
	#print('(IU event '+event_type+' '+str(iu.uid)+')')
	pass

class IpaacaIUStoreTestCase(unittest.TestCase):
	def setUp(self):
		self.ib = ipaaca.InputBuffer('TestIn', ['sensorcategory'])
		self.ib.register_handler(handle_iu_event)
		self.ob = ipaaca.OutputBuffer('TestOut')
		self.sensor_iu = ipaaca.IU('sensorcategory')
		self.sensor_iu.payload = {'data': 'sensordata'}
		time.sleep(0.1)
		self.ob.add(self.sensor_iu)
		time.sleep(0.1)
	def tearDown(self):
		pass
	def testInputBufferContents(self):
		hc.assert_that(self.ib.iu_store, hc.has_key(self.sensor_iu.uid))
		self.assertEqual(len(self.ib.iu_store), 1)
	def testOutputBufferContents(self):
		hc.assert_that(self.ib.iu_store, hc.has_key(self.sensor_iu.uid))
		self.assertEqual(len(self.ob.iu_store), 1)

class IpaacaPayloadTestCase(unittest.TestCase):
	def setUp(self):
		self.ib = ipaaca.InputBuffer('TestIn', ['sensorcategory', 'decisioncategory'])
		self.ob = ipaaca.OutputBuffer('TestOut')
		self.sensor_iu = ipaaca.IU('sensorcategory')
		self.sensor_iu.payload = {'data': 'sensordata'}
		self.ob.add(self.sensor_iu)
		
	def testPayloadContent(self):
		time.sleep(0.1)
		iu_received = self.ib.iu_store.get(self.sensor_iu.uid)
		self.assertEqual(iu_received.payload["data"], 'sensordata')


class IpaacaCommitTestCases(unittest.TestCase):

	def setUp(self):
		self.ib = ipaaca.InputBuffer('TestIn', ['sensorcategory'])
		self.ob = ipaaca.OutputBuffer('TestOut')
		self.iu = ipaaca.IU('sensorcategory')

	def testCommitBeforePublish(self):
		self.iu.commit()
		self.ob.add(self.iu)
		time.sleep(0.1)
		received_iu = self.ib.iu_store[self.iu.uid]
		self.assertTrue(received_iu.committed)

	def testCommitAfterPublish(self):
		self.ob.add(self.iu)
		self.iu.commit()
		time.sleep(0.1)
		received_iu = self.ib.iu_store[self.iu.uid]
		self.assertTrue(received_iu.committed)
	
	def testCommitAndLocalWrite(self):
		self.ob.add(self.iu)
		time.sleep(0.1)
		self.iu.commit()
		try:
			self.iu.payload['data'] = 'updatedData'
			self.fail("Expected an IUCommittedError but it was not raised.")
		except ipaaca.IUCommittedError, e:
			pass
	
	def testCommitAndRemoteWrite(self):
		self.ob.add(self.iu)
		self.iu.commit()
		time.sleep(0.1)
		received_iu = self.ib.iu_store[self.iu.uid]
		try:
			received_iu.payload['data'] = 'updatedData'
			self.fail("Expected an IUCommittedError but it was not raised.")
		except ipaaca.IUCommittedError, e:
			pass


class IpaacaLinksTestCase(unittest.TestCase):
	def setUp(self):
		self.ib = ipaaca.InputBuffer('TestIn', ['sensorcategory', 'decisioncategory'])
		self.ob = ipaaca.OutputBuffer('TestOut')
		self.sensor_iu = ipaaca.IU('sensorcategory')
		self.sensor_iu.payload = {'data': 'sensordata'}
		self.ob.add(self.sensor_iu)
	def tearDown(self):
		pass
	def testSetSingleLink(self):
		time.sleep(0.1)
		self.decision_iu = ipaaca.IU('decisioncategory')
		self.decision_iu.payload = {'data':'decision'}
		self.decision_iu.set_links( { 'grin': [self.sensor_iu.uid] } )
		self.ob.add(self.decision_iu)
		time.sleep(0.1)
		# test received version
		hc.assert_that(self.ib.iu_store, hc.has_key(self.decision_iu.uid))
		received_iu = self.ib.iu_store[self.decision_iu.uid]
		grinlinks = received_iu.get_links('grin')
		hc.assert_that(grinlinks, hc.has_item(self.sensor_iu.uid))
		self.assertEqual(len(grinlinks), 1)
	def testSetAndRemoveSingleLink(self):
		time.sleep(0.1)
		self.decision_iu = ipaaca.IU('decisioncategory')
		self.decision_iu.payload = {'data':'decision'}
		self.decision_iu.set_links( { 'grin': [self.sensor_iu.uid] } )
		self.ob.add(self.decision_iu)
		time.sleep(0.1)
		self.decision_iu.remove_links('grin', [self.sensor_iu.uid])
		time.sleep(0.1)
		# test received version
		hc.assert_that(self.ib.iu_store, hc.has_key(self.decision_iu.uid))
		received_iu = self.ib.iu_store[self.decision_iu.uid]
		grinlinks = received_iu.get_links('grin')
		self.assertEqual(len(grinlinks), 0)


class IpaacaRemoteWriteTestCase(unittest.TestCase):
	def setUp(self):
		self.ib = ipaaca.InputBuffer('TestIn', ['sensorcategory'])
		self.ib.register_handler(handle_iu_event)
		self.ob = ipaaca.OutputBuffer('TestOut')
		self.iu = ipaaca.IU('sensorcategory')
		self.iu.payload = {'data': 'sensordata'}
		time.sleep(0.1)
		self.ob.add(self.iu)
		time.sleep(0.1)
	def tearDown(self):
		pass
	def testRemotePayloadChange(self):
		hc.assert_that(self.ib.iu_store, hc.has_key(self.iu.uid))
		received_iu = self.ib.iu_store[self.iu.uid]
		received_iu.payload['data'] = 'updatedData'
		time.sleep(0.1)
		self.assertEqual(self.iu.payload['data'], 'updatedData')
	def testRemotePayloadReplace(self):
		hc.assert_that(self.ib.iu_store, hc.has_key(self.iu.uid))
		received_iu = self.ib.iu_store[self.iu.uid]
		received_iu.payload = { 'key1': 'value1', 'key2': 'value2' }
		time.sleep(0.1)
		self.assertEqual(len(self.iu.payload), 2)
		self.assertEqual(self.iu.payload['key1'], 'value1')
		self.assertEqual(self.iu.payload['key2'], 'value2')


if __name__ == '__main__':
	unittest.main()


package ipaaca;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.Matchers.any;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import ipaaca.protobuf.Ipaaca.IUPayloadUpdate;

import org.junit.Before;
import org.junit.Test;
/**
 * Unit testcases for the LocalIU
 * @author hvanwelbergen
 */
public class LocalIUTest
{
    OutputBuffer mockBuffer = mock(OutputBuffer.class);

    @Before
    public void setup()
    {
        when(mockBuffer.getUniqueName()).thenReturn("namex");
        when(mockBuffer.getOwningComponentName()).thenReturn("comp1");
    }

    @Test
    public void testCommit()
    {
        LocalIU liu = new LocalIU();
        liu.getPayload().put("key1", "item1");
        liu.setBuffer(mockBuffer);
        liu.commit("commitWriter");

        assertTrue(liu.isCommitted());
        verify(mockBuffer, times(1)).sendIUCommission(liu, "commitWriter");
    }

    @Test
    public void testSetPayloadOnUnpublishedIU()
    {
        LocalIU liu = new LocalIU();
        liu.getPayload().put("key1", "item1");
        assertEquals("item1", liu.getPayload().get("key1"));
    }

    @Test
    public void testSetPayloadOnPublishedIU()
    {
        LocalIU liu = new LocalIU();
        liu.setBuffer(mockBuffer);
        liu.getPayload().put("key1", "item1");
        assertEquals("item1", liu.getPayload().get("key1"));
        verify(mockBuffer, times(1)).sendIUPayloadUpdate(eq(liu), (IUPayloadUpdate) any());
    }

}

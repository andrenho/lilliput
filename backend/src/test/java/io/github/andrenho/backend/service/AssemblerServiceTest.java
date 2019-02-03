package io.github.andrenho.backend.service;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

import static org.junit.jupiter.api.Assertions.*;

@SpringBootTest
class AssemblerServiceTest {

    @Autowired
    private AssemblerService service;

    @Test
    public void success() {
        long id = service.createRequest(".SECTION text\nnop");
        while (service.requestStatus(id) != RequestStatus.Compiled)
            assertNotEquals(service.requestStatus(id), RequestStatus.Error);
        assertArrayEquals(service.requestResult(id), new byte[] { (byte) 0x7B });
    }

    @Test
    public void error() {
        long id = service.createRequest(".SECTION text\nnp");
        while (service.requestStatus(id) != RequestStatus.Error)
            assertNotEquals(service.requestStatus(id), RequestStatus.Compiled);
        System.out.println(service.requestError(id));
        assertTrue(service.requestError(id).length() > 0);
    }
}
package io.github.andrenho.backend.service;

import org.springframework.stereotype.Service;

import java.util.HashMap;
import java.util.Map;

@Service
public class AssemblerService {

    public long createRequest(String... sources) {
        Request req = new Request();
        requests.put(current_id++, req);
        req.execute(sources);
        return current_id - 1;
    }

    public RequestStatus requestStatus(long id) {
        return requests.get(id).getStatus();
    }

    public byte[] requestResult(long id) {
        Request r = requests.get(id);
        if (r.getStatus() == RequestStatus.Compiled)
            return r.getResult();
        else
            throw new RuntimeException("Invalid status for request.");
    }

    public String requestError(long id) {
        Request r = requests.get(id);
        if (r.getStatus() == RequestStatus.Error)
            return r.getError();
        else
            throw new RuntimeException("Invalid status for request.");
    }

    public void removeRequest(long id) {
        requests.get(id).stopProcessing();
        requests.remove(id);
    }

    private Map<Long, Request> requests = new HashMap<>();
    private long current_id = 0;
}

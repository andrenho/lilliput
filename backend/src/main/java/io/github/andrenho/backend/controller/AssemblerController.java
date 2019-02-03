package io.github.andrenho.backend.controller;

import io.github.andrenho.backend.service.AssemblerService;
import io.github.andrenho.backend.service.RequestStatus;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.*;

@ResponseStatus(value = HttpStatus.BAD_REQUEST)
class ResultNotAvailable extends RuntimeException {};

@RestController("/assembler")
public class AssemblerController {

    @Autowired
    private AssemblerService service;

    @PostMapping(value = "/request", produces = MediaType.APPLICATION_JSON_VALUE)
    public long createRequest(@RequestParam("sources") String[] sources) {
        return service.createRequest(sources);
    }

    @GetMapping(value = "/request/{id}/status", produces = MediaType.APPLICATION_JSON_VALUE)
    public RequestStatus requestStatus(@PathVariable long id) {
        return service.requestStatus(id);
    }

    @GetMapping(value = "/request/{id}/result", produces = MediaType.APPLICATION_OCTET_STREAM_VALUE)
    public byte[] result(@PathVariable long id) {
        return service.requestResult(id);
    }

    @GetMapping(value = "/request/{id}/error", produces = MediaType.APPLICATION_JSON_VALUE)
    public String error(@PathVariable long id) {
        return service.requestError(id);
    }

    @GetMapping(value = "/request/{id}")
    public void deleteRequest(@PathVariable long id) {
        service.removeRequest(id);
    }
}

package io.github.andrenho.backend.service;

import io.github.andrenho.backend.assembler.Assembler;
import io.github.andrenho.backend.assembler.CompilationError;
import io.github.andrenho.backend.assembler.CompiledCode;
import io.github.andrenho.backend.assembler.Linker;

class Executor implements Runnable {
    public Executor(Request request, String[] sources) {
        this.request = request;
        this.sources = sources;
    }

    private Request request;
    private String[] sources;

    @Override
    public void run() {
        try {
            CompiledCode cc[] = new CompiledCode[sources.length];
            int i = 0;
            for (String source : sources)
                cc[i++] = Assembler.compile(source);
            byte[] result = Linker.link(cc);
            request.setResult(result);
        } catch (CompilationError e) {
            request.setError(e.getMessage());
        }
    }
}

public class Request {
    public void execute(String[] sources) {
        status = RequestStatus.Compiling;
        Runnable r = new Executor(this, sources);
        new Thread(r).start();
    }

    public void stopProcessing() {
    }

    public synchronized RequestStatus getStatus() {
        return status;
    }

    public synchronized byte[] getResult() {
        return result;
    }

    public synchronized void setResult(byte[] result) {
        status = RequestStatus.Compiled;
        this.result = result;
    }

    public synchronized String getError() {
        return error;
    }

    public synchronized void setError(String error) {
        status = RequestStatus.Error;
        this.error = error;
    }

    private RequestStatus status = RequestStatus.NotInitialized;
    private byte[] result;
    private String error;
    private Thread thread;
}

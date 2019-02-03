package io.github.andrenho.backend.assembler;

@SuppressWarnings({"WeakerAccess", "unused"})
public class CompilationError extends Exception {
    public CompilationError() {
        super();
    }

    public CompilationError(String message) {
        super(message);
    }

    public CompilationError(String message, Throwable cause) {
        super(message, cause);
    }

    public CompilationError(Throwable cause) {
        super(cause);
    }

    protected CompilationError(String message, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
        super(message, cause, enableSuppression, writableStackTrace);
    }
}

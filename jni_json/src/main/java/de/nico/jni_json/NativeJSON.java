package de.nico.jni_json;

public final class NativeJSON {

    static {
        System.loadLibrary("native-lib");
    }

    public static native Object decode(String json);
    public static native <T> String encode(T json);

    public static <T> T decode(String json, Class<T> clazz) {
        return clazz.cast(decode(json));
    }
}
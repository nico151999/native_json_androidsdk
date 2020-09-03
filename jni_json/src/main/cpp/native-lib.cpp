#include <jni.h>
#include <string>

#include "zzzjson.h"

void jsonToString(jobject srco, JNIEnv* env, Value* desv,
        jclass lclazz, jclass mclazz, jclass iclazz, jclass dclazz, jclass sclazz, jclass bclazz,
        jmethodID mbooleanvalue, jmethodID mtostringdouble, jmethodID mtostringint, jmethodID mgetkey,
        jmethodID mgetvalue, jmethodID miteratorset, jmethodID miteratorlist, jmethodID mentryset,
        jmethodID mhasnext, jmethodID mnext) {
    if (env->IsInstanceOf(srco, lclazz)) {
        SetArray(desv);
        jobject oiterator = env->CallObjectMethod(srco, miteratorlist);
        while (env->CallBooleanMethod(oiterator, mhasnext)) {
            jobject oentry = env->CallObjectMethod(oiterator, mnext);
            Value *v = NewValue(desv->A);
            jsonToString(oentry, env, v, lclazz, mclazz, iclazz, dclazz, sclazz, bclazz, mbooleanvalue,
                         mtostringdouble, mtostringint, mgetkey, mgetvalue, miteratorset, miteratorlist,
                         mentryset, mhasnext, mnext);
            if (ArrayAddFast(desv, v) != True) return;
        }
    } else if (env->IsInstanceOf(srco, mclazz)) {
        SetObj(desv);
        jobject oset = env->CallObjectMethod(srco, mentryset);
        jobject oiterator = env->CallObjectMethod(oset, miteratorset);
        while (env->CallBooleanMethod(oiterator, mhasnext)) {
            jobject oentry = env->CallObjectMethod(oiterator, mnext);
            Value *v = NewValue(desv->A);
            SetKeyFast(
                    v,
                    env->GetStringUTFChars((jstring) env->CallObjectMethod(oentry, mgetkey), nullptr));
            jsonToString(env->CallObjectMethod(oentry, mgetvalue), env, v, lclazz, mclazz, iclazz,
                    dclazz, sclazz, bclazz, mbooleanvalue, mtostringdouble, mtostringint, mgetkey,
                    mgetvalue, miteratorset, miteratorlist, mentryset, mhasnext, mnext);
            if (ObjAddFast(desv, v) != True) return;
        }
    } else if (env->IsInstanceOf(srco, bclazz)) {
        const zzz_BOOL *b = env->CallBooleanMethod(srco, mbooleanvalue) ? &zzz_True : &zzz_False;
        if (b == nullptr)
            return;
        SetBool(desv, *b);
    } else if (env->IsInstanceOf(srco, sclazz)) {
        const char *str = env->GetStringUTFChars((jstring) srco, nullptr);
        if (str == nullptr)
            return;
        if (SetStrFast(desv, str) != True)
            return;
    } else if (env->IsInstanceOf(srco, iclazz)) {
        const char *str = env->GetStringUTFChars((jstring) env->CallObjectMethod(srco, mtostringint), nullptr);
        if (str == nullptr)
            return;
        if (SetNumStrFast(desv, str) != True)
            return;
    } else if (env->IsInstanceOf(srco, dclazz)) {
        const char *str = env->GetStringUTFChars((jstring) env->CallObjectMethod(srco, mtostringdouble), nullptr);
        if (str == nullptr)
            return;
        if (SetNumStrFast(desv, str) != True)
            return;
    }
}

jobject stringToJSON(Value *srcv, JNIEnv *env,
        jclass mclazz, jmethodID minit, jmethodID put,
        jclass lclazz, jmethodID linit, jmethodID add,
        jclass bclazz, jmethodID bvalue,
        jclass sclazz, jmethodID sinit,
        jclass iclazz, jmethodID ivalue,
        jclass dclazz, jmethodID dvalue) {
    const JSONType *t = Type(srcv);
    if (t == nullptr) {
        return nullptr;
    }
    switch (*t)
    {
        case JSONTYPEARRAY:
        {
            jobject list = env->NewObject(lclazz, linit);
            Value *next = Begin(srcv);
            while (next != nullptr) {
                env->CallBooleanMethod(
                        list,
                        add,
                        stringToJSON(
                                next, env,
                                mclazz, minit, put,
                                lclazz, linit, add,
                                bclazz, bvalue,
                                sclazz, sinit,
                                iclazz, ivalue,
                                dclazz, dvalue));
                next = Next(next);
            }
            return list;
        }
        case JSONTYPEOBJECT:
        {
            jobject map = env->NewObject(mclazz, minit);
            Value *next = Begin(srcv);
            while (next != nullptr) {
                env->CallObjectMethod(
                        map,
                        put,
                        env->NewStringUTF(GetKey(next)),
                        stringToJSON(
                                next, env,
                                mclazz, minit, put,
                                lclazz, linit, add,
                                bclazz, bvalue,
                                sclazz, sinit,
                                iclazz, ivalue,
                                dclazz, dvalue));
                next = Next(next);
            }
            return map;
        }
        case JSONTYPEBOOL:
        {
            return env->CallStaticObjectMethod(bclazz, bvalue, *GetBool(srcv));
        }
        case JSONTYPENULL:
        {
            return nullptr;
        }

        case JSONTYPESTRING:
        {
            return env->NewObject(sclazz, sinit, env->NewStringUTF(GetStr(srcv)));
        }
        case JSONTYPENUMBER:
        {
            const char* number = GetNumStr(srcv);
            if (strchr(number, '.') == nullptr) {
                return env->CallStaticObjectMethod(iclazz, ivalue, atoi(number));
            } else {
                return env->CallStaticObjectMethod(dclazz, dvalue, atof(number));
            }
        }
    }
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_de_nico_jni_1json_NativeJSON_decode(JNIEnv *env, jclass clazz, jstring json) {
    jclass mclazz = env->FindClass("java/util/HashMap");
    jmethodID minit = env->GetMethodID(mclazz, "<init>", "()V");
    jmethodID put = env->GetMethodID(mclazz, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    jclass lclazz = env->FindClass("java/util/ArrayList");
    jmethodID linit = env->GetMethodID(lclazz, "<init>", "()V");
    jmethodID add = env->GetMethodID(lclazz, "add", "(Ljava/lang/Object;)Z");

    jclass bclazz = env->FindClass("java/lang/Boolean");
    jmethodID bvalue = env->GetStaticMethodID(bclazz, "valueOf", "(Z)Ljava/lang/Boolean;");

    jclass sclazz = env->FindClass("java/lang/String");
    jmethodID sinit = env->GetMethodID(sclazz, "<init>", "(Ljava/lang/String;)V");

    jclass iclazz = env->FindClass("java/lang/Integer");
    jmethodID ivalue = env->GetStaticMethodID(iclazz, "valueOf", "(I)Ljava/lang/Integer;");

    jclass dclazz = env->FindClass("java/lang/Double");
    jmethodID dvalue = env->GetStaticMethodID(dclazz, "valueOf", "(D)Ljava/lang/Double;");

    Allocator* A = NewAllocator();
    Value* src_v = NewValue(A);
    const char* jsonString = env->GetStringUTFChars(json, nullptr);
    BOOL ret = ParseFast(src_v, jsonString);
    if (ret != True) {
        return nullptr;
    }
    jobject obj = stringToJSON(src_v, env, mclazz, minit, put, lclazz, linit, add, bclazz, bvalue, sclazz, sinit, dclazz, dvalue, iclazz, ivalue);
    ReleaseAllocator(A);

    return obj;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_de_nico_jni_1json_NativeJSON_encode(JNIEnv *env, jclass clazz, jobject json) {
    jclass lclazz = env->FindClass("java/util/List");
    jclass mclazz = env->FindClass("java/util/Map");
    jclass itclazz = env->FindClass("java/util/Iterator");
    jclass seclazz = env->FindClass("java/util/Set");
    jclass eclazz = env->FindClass("java/util/Map$Entry");
    jclass iclazz = env->FindClass("java/lang/Integer");
    jclass dclazz = env->FindClass("java/lang/Double");
    jclass sclazz = env->FindClass("java/lang/String");
    jclass bclazz = env->FindClass("java/lang/Boolean");
    jmethodID mbooleanvalue = env->GetMethodID(bclazz, "booleanValue", "()Z");
    jmethodID mtostringdouble = env->GetMethodID(dclazz, "toString", "()Ljava/lang/String;");
    jmethodID mtostringint = env->GetMethodID(iclazz, "toString", "()Ljava/lang/String;");
    jmethodID mgetkey = env->GetMethodID(eclazz, "getKey", "()Ljava/lang/Object;");
    jmethodID mgetvalue = env->GetMethodID(eclazz, "getValue", "()Ljava/lang/Object;");
    jmethodID miteratorset = env->GetMethodID(seclazz, "iterator", "()Ljava/util/Iterator;");
    jmethodID miteratorlist = env->GetMethodID(lclazz, "iterator", "()Ljava/util/Iterator;");
    jmethodID mentryset = env->GetMethodID(mclazz, "entrySet","()Ljava/util/Set;");
    jmethodID mhasnext = env->GetMethodID(itclazz, "hasNext", "()Z");
    jmethodID mnext = env->GetMethodID(itclazz, "next", "()Ljava/lang/Object;");

    Allocator *A = NewAllocator();
    Value *des_v = NewValue(A);
    jsonToString(json, env, des_v, lclazz, mclazz, iclazz, dclazz, sclazz, bclazz, mbooleanvalue,
            mtostringdouble, mtostringint, mgetkey, mgetvalue, miteratorset, miteratorlist, mentryset,
            mhasnext, mnext);
    ReleaseAllocator(A);
    return env->NewStringUTF(Stringify(des_v));
}
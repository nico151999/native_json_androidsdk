#include <jni.h>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

rapidjson::Value jobjecttojson(jobject *srco, JNIEnv *env, rapidjson::Document *doc, jclass *lclazz,
                               jclass *mclazz, jclass *iclazz, jclass *dclazz, jclass *sclazz,
                               jclass *bclazz, jmethodID *mbooleanvalue, jmethodID *mdoublevalue,
                               jmethodID *mgetkey, jmethodID *mgetvalue, jmethodID *miteratorset,
                               jmethodID *miteratorlist, jmethodID *mentryset, jmethodID *mhasnext,
                               jmethodID *mnext, jmethodID *mstringlength, jmethodID *mintvalue) {
    rapidjson::Value v;
    if (env->IsInstanceOf(*srco, *lclazz)) {
        v.SetArray();
        jobject oiterator = env->CallObjectMethod(*srco, *miteratorlist);
        while (env->CallBooleanMethod(oiterator, *mhasnext)) {
            jobject oentry = env->CallObjectMethod(oiterator, *mnext);
            v.PushBack(
                    jobjecttojson(&oentry, env, doc, lclazz, mclazz, iclazz, dclazz, sclazz, bclazz,
                                  mbooleanvalue, mdoublevalue, mgetkey, mgetvalue, miteratorset,
                                  miteratorlist, mentryset, mhasnext, mnext, mstringlength, mintvalue),
                    doc->GetAllocator());
        }
    } else if (env->IsInstanceOf(*srco, *mclazz)) {
        v.SetObject();
        jobject oset = env->CallObjectMethod(*srco, *mentryset);
        jobject oiterator = env->CallObjectMethod(oset, *miteratorset);
        while (env->CallBooleanMethod(oiterator, *mhasnext)) {
            jobject oentry = env->CallObjectMethod(oiterator, *mnext);
            env->CallObjectMethod(oentry, *mgetkey);
            env->CallObjectMethod(oentry, *mgetvalue);
            v.AddMember(
                    rapidjson::Value(*env->GetStringUTFChars(
                            (jstring) env->CallObjectMethod(oentry, *mgetkey), nullptr)),
                    jobjecttojson(&oentry, env, doc, lclazz, mclazz, iclazz, dclazz, sclazz, bclazz,
                                  mbooleanvalue, mdoublevalue, mgetkey, mgetvalue, miteratorset,
                                  miteratorlist, mentryset, mhasnext, mnext, mstringlength, mintvalue),
                    doc->GetAllocator()
            );
        }
    } else if (env->IsInstanceOf(*srco, *bclazz)) {
        v.SetBool(env->CallBooleanMethod(*srco, *mbooleanvalue));
    } else if (env->IsInstanceOf(*srco, *sclazz)) {
        v.SetString(
                env->GetStringUTFChars((jstring) srco, nullptr),
                env->CallIntMethod(*srco, *mstringlength, nullptr));
    } else if (env->IsInstanceOf(*srco, *iclazz)) {
        v.SetInt(env->CallIntMethod(*srco, *mintvalue, nullptr));
    } else if (env->IsInstanceOf(*srco, *dclazz)) {
        v.SetDouble(env->CallDoubleMethod(*srco, *mdoublevalue, nullptr));
    } // todo: was passiert wenn einer davon null ist?
    return v;
}

jobject jstringtojobject(rapidjson::Value *srcv, JNIEnv *env,
                         jclass *mclazz, jmethodID *minit, jmethodID *put,
                         jclass *lclazz, jmethodID *linit, jmethodID *add,
                         jclass *bclazz, jmethodID *bvalue,
                         jclass *sclazz, jmethodID *sinit,
                         jclass *iclazz, jmethodID *ivalue,
                         jclass *dclazz, jmethodID *dvalue,
                         jclass *loclazz, jmethodID *lovalue,
                         jclass *fclazz, jmethodID *fvalue) {
    rapidjson::Type t = srcv->GetType();
    switch (t) {
        case rapidjson::kArrayType: {
            jobject list = env->NewObject(*lclazz, *linit);
            for (auto &v : srcv->GetArray()) {
                env->CallBooleanMethod(
                        list,
                        *add,
                        jstringtojobject(
                                &v, env,
                                mclazz, minit, put,
                                lclazz, linit, add,
                                bclazz, bvalue,
                                sclazz, sinit,
                                iclazz, ivalue,
                                dclazz, dvalue,
                                loclazz, lovalue,
                                fclazz, fvalue));
            }
            return list;
        }
        case rapidjson::kObjectType: {
            jobject map = env->NewObject(*mclazz, *minit);
            for (auto &v : srcv->GetObject()) {
                env->CallObjectMethod(
                        map,
                        *put,
                        env->NewStringUTF(v.name.GetString()),
                        jstringtojobject(
                                &v.value, env,
                                mclazz, minit, put,
                                lclazz, linit, add,
                                bclazz, bvalue,
                                sclazz, sinit,
                                iclazz, ivalue,
                                dclazz, dvalue,
                                loclazz, lovalue,
                                fclazz, fvalue));
            }
            return map;
        }
        case rapidjson::kTrueType: {
            return env->CallStaticObjectMethod(*bclazz, *bvalue, true);
        }
        case rapidjson::kFalseType: {
            return env->CallStaticObjectMethod(*bclazz, *bvalue, false);
        }
        case rapidjson::kNullType: {
            return nullptr;
        }
        case rapidjson::kStringType: {
            return env->NewObject(*sclazz, *sinit, env->NewStringUTF(srcv->GetString()));
        }
        case rapidjson::kNumberType: {
            if (srcv->IsInt()) {
                return env->CallStaticObjectMethod(*iclazz, *ivalue, srcv->GetInt());
            }
            if (srcv->IsUint()) {
                return env->CallStaticObjectMethod(*iclazz, *ivalue, srcv->GetUint());
            }
            if (srcv->IsInt64()) {
                return env->CallStaticObjectMethod(*loclazz, *lovalue, srcv->GetInt64());
            }
            if (srcv->IsUint64()) {
                return env->CallStaticObjectMethod(*loclazz, *lovalue, srcv->GetUint64());
            }
            if (srcv->IsFloat()) {
                return env->CallStaticObjectMethod(*fclazz, *fvalue, srcv->GetFloat());
            }
            if (srcv->IsLosslessFloat()) {
                return env->CallStaticObjectMethod(*fclazz, *fvalue, srcv->GetFloat());
            }
            if (srcv->IsDouble()) {
                return env->CallStaticObjectMethod(*dclazz, *dvalue, srcv->GetDouble());
            }
            if (srcv->IsLosslessDouble()) {
                return env->CallStaticObjectMethod(*dclazz, *dvalue, srcv->GetDouble());
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
    jmethodID put = env->GetMethodID(mclazz, "put","(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    jclass lclazz = env->FindClass("java/util/ArrayList");
    jmethodID linit = env->GetMethodID(lclazz, "<init>", "()V");
    jmethodID add = env->GetMethodID(lclazz, "add", "(Ljava/lang/Object;)Z");

    jclass bclazz = env->FindClass("java/lang/Boolean");
    jmethodID bvalue = env->GetStaticMethodID(bclazz, "valueOf", "(Z)Ljava/lang/Boolean;");

    jclass sclazz = env->FindClass("java/lang/String");
    jmethodID sinit = env->GetMethodID(sclazz, "<init>", "(Ljava/lang/String;)V");

    jclass iclazz = env->FindClass("java/lang/Integer");
    jmethodID ivalue = env->GetStaticMethodID(iclazz, "valueOf", "(I)Ljava/lang/Integer;");

    jclass loclazz = env->FindClass("java/lang/Long");
    jmethodID lovalue = env->GetStaticMethodID(loclazz, "valueOf", "(J)Ljava/lang/Long;");

    jclass dclazz = env->FindClass("java/lang/Double");
    jmethodID dvalue = env->GetStaticMethodID(dclazz, "valueOf", "(D)Ljava/lang/Double;");

    jclass fclazz = env->FindClass("java/lang/Float");
    jmethodID fvalue = env->GetStaticMethodID(fclazz, "valueOf", "(F)Ljava/lang/Float;");

    const char *jsonString = env->GetStringUTFChars(json, nullptr);
    rapidjson::Document d;
    d.Parse(jsonString);
    return jstringtojobject(&d, env, &mclazz, &minit, &put, &lclazz, &linit, &add, &bclazz, &bvalue,
                                   &sclazz, &sinit, &dclazz, &dvalue, &iclazz, &ivalue, &loclazz, &lovalue,
                                   &fclazz, &fvalue);
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
    jmethodID mdoublevalue = env->GetMethodID(dclazz, "doubleValue", "()D");
    jmethodID mintvalue = env->GetMethodID(iclazz, "intValue", "()I");
    jmethodID mgetkey = env->GetMethodID(eclazz, "getKey", "()Ljava/lang/Object;");
    jmethodID mgetvalue = env->GetMethodID(eclazz, "getValue", "()Ljava/lang/Object;");
    jmethodID miteratorset = env->GetMethodID(seclazz, "iterator", "()Ljava/util/Iterator;");
    jmethodID miteratorlist = env->GetMethodID(lclazz, "iterator", "()Ljava/util/Iterator;");
    jmethodID mentryset = env->GetMethodID(mclazz, "entrySet", "()Ljava/util/Set;");
    jmethodID mhasnext = env->GetMethodID(itclazz, "hasNext", "()Z");
    jmethodID mnext = env->GetMethodID(itclazz, "next", "()Ljava/lang/Object;");
    jmethodID mstringlength = env->GetMethodID(sclazz, "length", "()I");

    rapidjson::Document d;
    rapidjson::Value v = jobjecttojson(&json, env, &d, &lclazz, &mclazz, &iclazz, &dclazz,
                                       &sclazz, &bclazz, &mbooleanvalue,
                                       &mdoublevalue, &mgetkey, &mgetvalue,
                                       &miteratorset, &miteratorlist, &mentryset,
                                       &mhasnext, &mnext, &mstringlength, &mintvalue);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    v.Accept(writer);
    return env->NewStringUTF(buffer.GetString());
}
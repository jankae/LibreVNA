#ifndef QPOINTERVARIANT_H
#define QPOINTERVARIANT_H

#include <QVariant>

class QPointerVariant {
public:
    template <typename T> QPointerVariant(T *ptr)
        : ptr(static_cast<void*>(ptr)),
          variant(QVariant(*ptr)){};
    void setValue(const QVariant &value) {
        auto destType = variant.type();
        if(!value.canConvert(destType)) {
            throw std::runtime_error("Unable to convert QVariant to requested type");
        }
        variant = value;
        variant.convert(destType);
        QMetaType mt(destType);
        mt.construct(ptr, variant.constData());
    }
    QVariant value() {
        return QVariant(variant.type(), ptr);
    }
private:
    void *ptr;
    QVariant variant;
};

#endif // QPOINTERVARIANT_H

# Contributing to VoltronicMAX

شكراً لاهتمامك بالمساهمة!

## كيف تساهم

1. Fork: https://github.com/kitronic/esp-lib-voltronic-max
2. Branch: `git checkout -b feature/my-feature`
3. Commit: `git commit -am 'Add new feature'`
4. Push: `git push origin feature/my-feature`
5. Pull Request

## قواعد الكود

### قواعد أساسية
- **لا** تستعمل `String` — استعمل `char[]`
- **لا** تستعمل `delay()` داخل المكتبة
- **لا** تعمل dynamic allocation (`new`, `malloc`)
- اتبع نمط الأسماء الموجود
- أضف اختبار لكل ميزة جديدة

### v1.3.0 — قواعد إضافية
- كل النصوص العربية **بالإنجليزية في الكود** (في `VoltronicLang.cpp` فقط)
- استخدم `inverter.lang.tr(Key)` بدل نصوص مباشرة
- لكل نوع بطارية: منحنى `CURVE_*` في `VoltronicBattery.cpp`
- عند إضافة ميزة بـ EEPROM: أضف حقل في `VoltronicStorage::Data` + تحديث `_applyTo` و `_captureFrom`
- عند إضافة أمر: أضف في `VoltronicCommands.h` + parser + method في `VoltronicMAX`
- الترميز: **UTF-8 بدون BOM** لكل الملفات

### بنية الملفات (v1.3.0)
src/
├── VoltronicMAX.h/cpp ← API رئيسي
├── VoltronicConfig.h ← الإعدادات
├── VoltronicCommands.h ← ثوابت الأوامر
├── VoltronicCRC.h ← CRC
├── VoltronicTypes.h ← structs
├── VoltronicParser.h/cpp ← parsers
├── VoltronicTransport.h ← UART
├── VoltronicWeb.h ← Web Console (opt-in)
├── VoltronicBattery.h/cpp ← حساب البطارية (v1.3.0)
├── VoltronicSmartCharger.h/cpp ← شحن ذكي (v1.3.0)
├── VoltronicPowerMode.h/cpp ← وضع الطاقة (v1.3.0)
├── VoltronicStorage.h/cpp ← EEPROM (v1.3.0)
└── VoltronicLang.h/cpp ← ترجمة AR/EN (v1.3.0)


## الإبلاغ عن مشاكل

افتح Issue مع:
- نوع الإنفرتر والموديل
- إصدار الفيرموير (`queryFirmware()`)
- الكود المستعمل
- الرد الخام (`lastResponse()`)
- رسالة الخطأ (`lastError()`)
- **جديد:** نوع البطارية والسعة + إعدادات Power Mode / Smart Charger

## Pull Request Checklist

- [ ] الكود يبني بدون تحذيرات
- [ ] لا `String` / `delay()` / `new`
- [ ] Native tests تمر (`pio test -e native`)
- [ ] الأمثلة تـ compile
- [ ] الترميز UTF-8 بدون BOM
- [ ] Arabic strings في `VoltronicLang.cpp` فقط
- [ ] README / CHANGELOG محدّثين

## License

بالمساهمة، أنت توافق أن مساهمتك تحت رخصة MIT.
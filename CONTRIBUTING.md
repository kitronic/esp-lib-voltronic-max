# Contributing to VoltronicMAX

شكراً لاهتمامك بالمساهمة!

## كيف تساهم

1. Fork: https://github.com/kitronic/esp-lib-voltronic-max
2. Branch: `git checkout -b feature/my-feature`
3. Commit: `git commit -am 'Add new feature'`
4. Push: `git push origin feature/my-feature`
5. Pull Request

## قواعد الكود

- **لا** تستعمل `String` — استعمل `char[]`
- **لا** تستعمل `delay()` داخل المكتبة
- **لا** تعمل dynamic allocation (`new`, `malloc`)
- اتبع نمط الأسماء الموجود
- أضف اختبار لكل ميزة جديدة

## الإبلاغ عن مشاكل

افتح Issue مع:
- نوع الإنفرتر والموديل
- إصدار الفيرموير (`queryFirmware()`)
- الكود المستعمل
- الرد الخام (`lastResponse()`)
- رسالة الخطأ (`lastError()`)

## License

بالمساهمة، أنت توافق أن مساهمتك تحت رخصة MIT.
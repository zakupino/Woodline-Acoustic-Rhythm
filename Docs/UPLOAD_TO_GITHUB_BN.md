# GitHub-এ আপলোড করার ধাপ (বাংলা)

1. GitHub-এ New repository তৈরি করুন। নাম দিন `Woodline-Acoustic-Rhythm`।
2. এই ZIP-টি আগে কম্পিউটারে Extract করুন।
3. Repository-র Code পেজে **Add file → Upload files** নির্বাচন করুন। Extract করা ফোল্ডারের ভেতরের ফাইলগুলো আপলোড করুন; শুধু ZIP ফাইলটি আপলোড করবেন না।
4. `.github/workflows/build-vst3.yml` ফাইলের path যেন ঠিক থাকে, সেটি নিশ্চিত করুন। এটি না থাকলে Actions workflow চালু হবে না।
5. **Commit changes** চাপুন।
6. **Actions** ট্যাবে গিয়ে **Build Woodline VST3** খুলুন।
7. Windows-এর জন্য `Woodline-Windows-x64-VST3` Artifact ডাউনলোড করুন। Apple Silicon Mac হলে `Woodline-macOS-ARM64-VST3`, Intel Mac হলে `Woodline-macOS-Intel-x64-VST3` ডাউনলোড করুন।
8. Artifact ZIP Extract করুন। ভেতরের `Woodline Acoustic Rhythm.vst3` পুরো ফোল্ডারটি Windows-এ `C:\Program Files\Common Files\VST3`-এ কপি করুন।
9. Ableton Live → Settings/Preferences → Plug-ins → Rescan করুন।

**মনে রাখবেন:** এই ZIP-এ সম্পূর্ণ source project ও auto-build ব্যবস্থা আছে; আগেই compile করা Windows `.vst3` binary নেই। GitHub Actions build সফল হলে binary Artifact তৈরি হবে। Ableton-এ VST3 খুঁজে পেতে plug-in folder-এ একবার কপি করতেই হবে।

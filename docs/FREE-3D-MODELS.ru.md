# LAST CALL — Каталог бесплатных 3D-моделей

Подобранные **бесплатные** ассеты для нуар-игры про бар: интерьер бара, персонажи, ночные улицы, транспорт, текстуры и UE5-нативные источники. Тон: [PHILOSOPHY.ru.md](../PHILOSOPHY.ru.md), слоты контента: [GAME-DESIGN.ru.md](GAME-DESIGN.ru.md).

**Проверено:** 13.10.2026. Каждый URL ниже был запрошен в этом проходе (HTTP 200, либо 202 для JS-оболочки Sketchfab). Перепроверяйте лицензии перед релизом — бесплатно ≠ навсегда.

**[English version of this file →](FREE-3D-MODELS.md)**

## 1. Как читать каталог

- Колонка **License**: `CC0` — без указания автора, коммерция ок. `CC-BY` — нужно указание автора. `CC-BY-SA` — указание **+ share-alike** (избегать в закрытой игре). `Free` — бесплатно по собственной EULA платформы (Fab, Mixamo, Marketplace).
- Колонка **Format** — что даёт источник. Для Sketchfab проверьте переключатель *Downloadable* на странице модели — набор файлов задаёт загрузчик.
- **Rigged?** указан в Notes: `rigged` — есть скелет для скининга, `anim` — приложены клипы анимаций.
- Всё, что в Notes помечено `⚠`, требует решения человека до использования.

### Сводка лицензий (безопасность для коммерческой игры)

| Лицензия | Использование в LAST CALL | Атрибуция | Примечания |
| :--- | :--- | :--- | :--- |
| CC0 (Poly Haven, ambientCG, Quaternius, Kenney, Poly Pizza) | ✅ Бесплатно | Нет | Самый безопасный уровень — благодарить некого |
| CC-BY 4.0 (Sketchfab) | ✅ Бесплатно | Да, в титрах | Вести `CREDITS.txt` |
| CC-BY-SA (Sketchfab, пара позиций OGA) | ⚠ | Да + производное остаётся SA | Предпочитать CC-BY/CC0-альтернативы |
| CC-BY-NC / NC-ND / только GPL | ❌ Исключено | — | Не безопасно для коммерции; отфильтровано из списка |
| Fab / Marketplace / Mixamo EULA | ✅ Бесплатно | Нет (действует EULA) | Бесплатно *внутри* UE-проектов и релизов; для Mixamo FBX вне UE перечитайте [FAQ Mixamo](https://www.mixamo.com) |
| Вырезанные из игр ассеты | ❌ Исключено | — | Очевидные пиратские перезаливы отсеяны при сканировании |

---

## 2. Интерьер бара и реквизит

Стойка, бутылки, стекло, табуретки, полки, неон. Ядро наполнения заведения.

| Категория | Название | Источник | URL | Лицензия | Формат | Примечания |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Интерьер бара | Pub interior | Sketchfab | https://sketchfab.com/3d-models/none-a6534141c33343b382dd23d519c3de73 | CC-BY 4.0 | glTF / FBX / OBJ\* | Помещение целиком: стойка, бэк-бар, табуретки. Без рига. Лучшая база для блокаута |
| Барная стойка | bar counter | Sketchfab | https://sketchfab.com/3d-models/none-78c700db0d9f4d7c800fb789dfa2e057 | CC-BY 4.0 | glTF / FBX / OBJ\* | PBR-стойка отдельно — легко встаёт в модульную комнату |
| Барная стойка | BarCounter - 8192px² | Sketchfab | https://sketchfab.com/3d-models/none-f3646304fee941c19a6086d033ecd9d2 | CC-BY 4.0 | glTF / FBX / OBJ\* | Текстуры 8K — качество героя для камеры «налива» |
| Табуретка | Worn Bar Stool | Sketchfab | https://sketchfab.com/3d-models/none-587a6f10e74f4f8298f6ad15b42bb7cf | CC-BY 4.0 | glTF / FBX / OBJ\* | Потрёпанный вид — ложится в нуар-грязь |
| Табуретка | Bar Stool and Table | Sketchfab | https://sketchfab.com/3d-models/none-880b2ead4b19445896522bd9b6e46517 | CC-BY 4.0 | glTF / FBX / OBJ\* | Набор из двух — столы для рядов посадки |
| Полки | Wooden Back Bar Cabinet - 4096px² | Sketchfab | https://sketchfab.com/3d-models/none-a1961b9e5a7043fb9a70b855e947e27f | CC-BY 4.0 | glTF / FBX / OBJ\* | Бэк-бар 4K; стена позади Алекса |
| Неон | Neon Bar Sign | Sketchfab | https://sketchfab.com/3d-models/none-52d50a3b04ae414cb11b16df0d779054 | CC-BY 4.0 | glTF / FBX / OBJ\* | Emissive — вешать на UE emissive-материал + мерцание |
| Стекло | Cocktail Glass | Sketchfab | https://sketchfab.com/3d-models/none-4f7eeaf5583940d4a5368a431b5a2a8a | CC-BY 4.0 | glTF / FBX / OBJ\* | Бокал для мартини — напиток мисс Кейн |
| Стекло | Glass Collection | Sketchfab | https://sketchfab.com/3d-models/none-0ff8fc5007dd4e9da675109fe7c443d6 | CC-BY 4.0 | glTF / FBX / OBJ\* | Разное стекло в одном файле |
| Стекло | The ultimate glass pack | Sketchfab | https://sketchfab.com/3d-models/none-1ea37815fb0145e49487386d6924b341 | CC-BY 4.0 | glTF / FBX / OBJ\* | Самый большой набор бокалов — покрывает все типы заказов |
| Бутылки | Whiskey and Wine Low-Poly Bottles | Sketchfab | https://sketchfab.com/3d-models/none-99999f032f624e8cb05651f041dbf8eb | CC-BY 4.0 | glTF / FBX / OBJ\* | Low-poly — дёшево для стены из 40+ бутылок |
| Бутылки | Bar with bottles | Sketchfab | https://sketchfab.com/3d-models/none-acdb303e9849401f95a973469e0310c3 | CC-BY 4.0 | glTF / FBX / OBJ\* | Готовая полка — мгновенная отделка бэк-бара |
| Бутылки | Wine Bottles | Poly Haven | https://polyhaven.com/a/wine_bottles_01 | CC0 | FBX / OBJ / Blend / glTF | Без атрибуции. Фотореалистично — лучше для крупных планов |
| Барный инвентарь | Bar Shaker | Sketchfab | https://sketchfab.com/3d-models/none-ec515bceb65a4d7ea59da147a7ea0da7 | ⚠ CC-BY-SA 4.0 | glTF / FBX / OBJ\* | **Share-alike** — для релиза предпочесть CC0-альтернативу |
| Барный инвентарь | Molotov cocktail with lighter | Sketchfab | https://sketchfab.com/3d-models/none-11c68745873848dc8d6b15051d9e678e | CC-BY 4.0 | glTF / FBX / OBJ\* | Реквизит эскалации фракций |
| Реквизит | Cash Register | Poly Haven | https://polyhaven.com/a/CashRegister_01 | CC0 | FBX / OBJ / Blend / glTF | Касса на стойке |
| Мебель | bar_chair_round_01 | Poly Haven | https://polyhaven.com/a/bar_chair_round_01 | CC0 | FBX / OBJ / Blend / glTF | Круглый барный стул |
| Мебель | metal_stool_01 | Poly Haven | https://polyhaven.com/a/metal_stool_01 | CC0 | FBX / OBJ / Blend / glTF | Также `metal_stool_02`, `metal_stool_03` |
| Мебель | wooden_stool_01 | Poly Haven | https://polyhaven.com/a/wooden_stool_01 | CC0 | FBX / OBJ / Blend / glTF | Также `wooden_stool_02` |
| Мебель | Ultimate Furniture Pack | Quaternius | https://quaternius.com/packs/ultimatefurniture.html | CC0 | FBX / OBJ / Blend / glTF | Широкий набор интерьерного наполнения в одной загрузке |
| Мебель | Ultimate House Interior Pack | Quaternius | https://quaternius.com/packs/ultimatehomeinterior.html | CC0 | FBX / OBJ / Blend / glTF | Стены, двери, реквизит для модульной комнаты |
| Набор бара (стилизованный) | Stylized Low Poly Bar Counter Set with Stools and Shelves | Fab | https://www.fab.com/listings/ec950ae1-4bf5-4957-9ce0-b220932a34c2 | Free (EULA Fab) | FBX / OBJ / Blend | Стойка + полки + бутылки + стекло + табуретки, один кит. Low-poly/стилизация, не фотореализм |
| Мебель (OGA) | 3D Furniture | OpenGameArt | https://opengameart.org/content/3d-furniture | См. страницу | Blend / OBJ | Есть превью `Bar.png` — набор барной мебели |
| Мебель (OGA) | Tiki Bar Stall | OpenGameArt | https://opengameart.org/content/tiki-bar-stall | См. страницу | Blend / OBJ | Барная будка; полезна как силуэтный референс |
| Хранилище (OGA) | Big wine barrel | OpenGameArt | https://opengameart.org/content/big-wine-barrel | См. страницу | Blend / OBJ | Наполнение погреба/бункера |

\* Форматы загрузки Sketchfab различаются у загрузчиков — проверяйте страницу модели.

## 3. Персонажи (бармен, клиенты, детектив, мафия)

| Категория | Название | Источник | URL | Лицензия | Формат | Примечания |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Ригнутая модель в пальто | Man In Coat - Human Rigged Model | Sketchfab | https://sketchfab.com/3d-models/none-cd71038373d5427099a7970f1a1af2df | CC-BY 4.0 | glTF / FBX\* | **rigged**, 1 анимация. Силуэт пальто = читаемый детектив/мафия |
| Ригнутая модель в пальто | old man in coat - human riged model | Sketchfab | https://sketchfab.com/3d-models/none-e8052f5928ef42f1ae5b5af02eb1a36b | CC-BY 4.0 | glTF / FBX\* | **rigged**, 1 анимация. Старший клиент / дядя в флешбэках |
| Детектив | Character (Type Detective) | Sketchfab | https://sketchfab.com/3d-models/none-b2324023364c458db010fe39e8111f38 | CC-BY 4.0 | glTF / FBX\* | Визуально подходит Детективу Харрису |
| NPC (типовой) | rigged character low poly | Sketchfab | https://sketchfab.com/3d-models/none-aa2e761ce168492484f8483717898ec8 | CC-BY 4.0 | glTF / FBX\* | **rigged** — дешёвая заливка толпы |
| Мафия | Boss Mafia Man character | Sketchfab | https://sketchfab.com/3d-models/none-3bdb67fd567546f3bf22ada378fe8356 | CC-BY 4.0 | glTF / FBX\* | Босс фракции — прообраз «Тени» |
| Мафия | Mafia Boss with Gun – Stylized Noir Gangster | Sketchfab | https://sketchfab.com/3d-models/none-abdb14866d91403cab47feaa5393e2f5 | CC-BY 4.0 | glTF / FBX\* | Явно нуар-стиль; пистолет в комплекте |
| Мафия | English Gangster Character Peaky Blinders Style | Sketchfab | https://sketchfab.com/3d-models/none-e63bb61fa3d54d9b83f544043df30cce | CC-BY 4.0 | glTF / FBX\* | Гангстер эпохи — отсылка к 1920–30-м |
| Мафия | Yakuza | Sketchfab | https://sketchfab.com/3d-models/none-67cc7f612434409db2c3cba6a00294eb | CC-BY 4.0 | glTF / FBX\* | Облик третьей фракции |
| Толпа | 1970 Gangster Soldier | Sketchfab | https://sketchfab.com/3d-models/none-d902a608c8584f32b0fa11b67757ff4a | CC-BY 4.0 | glTF / FBX\* | ⚠ T-pose, **без рига** — нужен риггинг |
| Фоновый NPC | Night Stalker - male | Sketchfab | https://sketchfab.com/3d-models/none-35626d172abb420498ae5c42a9fea2fb | CC-BY 4.0 | glTF / FBX\* | **rigged**, 2 анимации. Уличный/чужой |
| Фоновый NPC | Night Stalker - female | Sketchfab | https://sketchfab.com/3d-models/none-22a9d24975f94e29909d2eff1e44fb40 | CC-BY 4.0 | glTF / FBX\* | **rigged**, 2 анимации |
| Базовое тело | Universal Base Characters | Quaternius | https://quaternius.com/packs/universalbasecharacters.html | CC0 | FBX / OBJ / Blend / glTF | **rigged**, ретаргетится — база для собственного Алекса |
| Анимации | Universal Animation Library | Quaternius | https://quaternius.com/packs/universalanimationlibrary.html | CC0 | FBX / Blend / glTF | Гуманоидный риг, ретаргет на UE-манекен |
| Анимации | Universal Animation Library 2 | Quaternius | https://quaternius.com/packs/universalanimationlibrary2.html | CC0 | FBX / Blend / glTF | Вторая партия — циклы idle/talk |
| Анимированный NPC | Animated Men Pack | Quaternius | https://quaternius.com/packs/animatedmen.html | CC0 | FBX / OBJ / Blend / glTF | Готовые анимированные клиенты |
| Анимированный NPC | Animated Women Pack | Quaternius | https://quaternius.com/packs/animatedwomen.html | CC0 | FBX / OBJ / Blend / glTF | Готовые анимированные клиенты |
| Анимированный NPC | Ultimate Animated Character Pack | Quaternius | https://quaternius.com/packs/ultimatedanimatedcharacter.html | CC0 | FBX / OBJ / Blend / glTF | Набор walk/idle в один клик |
| Толпа | Background Posed Humans Pack | Quaternius | https://quaternius.com/packs/backgroundposedhumans.html | CC0 | FBX / OBJ / Blend / glTF | Статичные сидя/стоя тела — только фон |
| Анимации | Mixamo (вся библиотека) | Adobe Mixamo | https://www.mixamo.com | Free (EULA Mixamo) | FBX | **rigged + anim** — ретаргет клипов idle/talk/drink на любой гуманоид. Бесплатно для коммерции |
| Толпа (OGA) | Animated Human Low Poly | OpenGameArt | https://opengameart.org/content/animated-human-low-poly | См. страницу | Blend / FBX | Low-poly анимированный человек для дальних NPC |

## 4. Ночные улицы и экстерьеры

| Категория | Название | Источник | URL | Лицензия | Формат | Примечания |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Блокировка улиц | Cyberpunk Game Kit | Quaternius | https://quaternius.com/packs/cyberpunkgamekit.html | CC0 | FBX / OBJ / Blend / glTF | **71 модель** — неоновый город, ближайшее совпадение по стилю с LAST CALL |
| Блокировка улиц | Downtown City MegaKit | Quaternius | https://quaternius.com/packs/downtowncitymegakit.html | CC0 | FBX / OBJ / Blend / glTF | Модульные городские здания |
| Блокировка улиц | Modular Streets Pack | Quaternius | https://quaternius.com/packs/modularstreets.html | CC0 | FBX / OBJ / Blend / glTF | Плиты дорог/тротуаров для экстерьера |
| Наполнение улиц | City Kit (Roads) | Kenney | https://kenney.nl/assets/city-kit-roads | CC0 | FBX / OBJ / Blend / glTF / SVG | 90 файлов — куски дорог, чисто и читаемо |
| Наполнение улиц | City Kit (Commercial) | Kenney | https://kenney.nl/assets/city-kit-commercial | CC0 | FBX / OBJ / Blend / glTF | Витрины/фасады для квартала вокруг бара |
| Наполнение улиц | City Kit (Industrial) | Kenney | https://kenney.nl/assets/city-kit-industrial | CC0 | FBX / OBJ / Blend / glTF | Наполнение промышленного переулка |
| Наполнение улиц | City Kit (Suburban) | Kenney | https://kenney.nl/assets/city-kit-suburban | CC0 | FBX / OBJ / Blend / glTF | Окраины / маршруты отхода |
| Уличная сцена | LOWPOLY CITY STREET PACK | Sketchfab | https://sketchfab.com/3d-models/none-8e1ba8a437c4460eaaa643953eaf79d0 | CC-BY 4.0 | glTF / FBX\* | Улица целиком — быстрый блокировка экстерьера |
| Уличная сцена | Cyberpunk City | Sketchfab | https://sketchfab.com/3d-models/none-3f24e5c5bf924f46b30d9a392afa9624 | CC-BY 4.0 | glTF / FBX\* | Ночной неоновый город — референс света + пригодная геометрия |
| Переулок | Alleyway | Sketchfab | https://sketchfab.com/3d-models/none-26cb4d9be2ea476686b5d463af642db5 | CC-BY 4.0 (Free Standard) | glTF / FBX\* | Задний переулок за баром |
| Неон | Neon Open Sign | Sketchfab | https://sketchfab.com/3d-models/none-2efad97b50f34220b2df92fe99c8a2ad | CC-BY 4.0 | glTF / FBX\* | OPEN/CLOSED — привязка к режиму работы бара |
| Неон | Game Ready Cocktails Neon Sign | Sketchfab | https://sketchfab.com/3d-models/none-1698da3aa0c94ac2bdbc0f38ad178e04 | CC-BY 4.0 | glTF / FBX\* | Неон с коктейлями — собственная вывеска бара |
| Неон (Fab) | Cyberpunk animated japanese LED neon sign | Fab | https://www.fab.com/listings/8a251ab5-a714-4101-9016-5a22624220dc | Free (EULA Fab) | См. страницу | Анимированный LED-неон, мерцание запечено |
| Неон (Fab) | CC0 - Neon Sign Open | Fab | https://www.fab.com/listings/4e687ce7-9aa0-43df-8868-c17da645bbeb | CC0 | См. страницу | Явно CC0 — без атрибуции |
| Уличная мебель | Street Lamp | Sketchfab | https://sketchfab.com/3d-models/none-6881aa1e84b047d79860fa9297e05e22 | CC-BY 4.0 | glTF / FBX\* | Источники ключевого света в мокрую ночь |
| Уличная мебель | Victorian Street Lamp | Sketchfab | https://sketchfab.com/3d-models/none-b7cebcfb4b1c4d4a97e22072800704ca | CC-BY 4.0 | glTF / FBX\* | Старый город — периодный нуар |
| Уличная мебель | Traffic Lights [Street Assets Vol. 02] | Sketchfab | https://sketchfab.com/3d-models/none-ab3efdd259a54d4aba5f878eacbe3f20 | CC-BY 4.0 | glTF / FBX\* | Наполнение перекрёстка |
| Уличная мебель | street_lamp_01 | Poly Haven | https://polyhaven.com/a/street_lamp_01 | CC0 | FBX / OBJ / Blend / glTF | Также `street_lamp_02` |
| Уличная мебель | hanging_industrial_lamp | Poly Haven | https://polyhaven.com/a/hanging_industrial_lamp | CC0 | FBX / OBJ / Blend / glTF | Подвес над переулком/в помещении |
| Уличная мебель | fire_hydrant | Poly Haven | https://polyhaven.com/a/fire_hydrant | CC0 | FBX / OBJ / Blend / glTF | Реквизит тротуара |
| Уличный реквизит | Debris Papers | Poly Pizza | https://poly.pizza/m/MujITy1NRR | CC0 | FBX / OBJ / glTF | Мусор на мокрых улицах |
| Уличный реквизит | Dumpster | Poly Pizza | https://poly.pizza/m/PKsbolkZSr | CC0 | FBX / OBJ / glTF | Реквизит переулка |
| Уличный реквизит | Traffic Light | Poly Pizza | https://poly.pizza/m/lg9AKWejnF | CC0 | FBX / OBJ / glTF | Low-poly вариант |
| Текстуры дорог | Road Textures | Kenney | https://kenney.nl/assets/road-textures | CC0 | PNG | Тайлящаяся разметка дорог |

## 5. Транспорт, полиция и оружие

| Категория | Название | Источник | URL | Лицензия | Формат | Примечания |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Полицейская машина | Police Car | Sketchfab | https://sketchfab.com/3d-models/none-5510d0c264a24e1990a578142813894e | CC-BY 4.0 | glTF / FBX\* | Основной патруль у бара |
| Полицейская машина | POLICE CAR - LOW POLY | Sketchfab | https://sketchfab.com/3d-models/none-0f5b12ea1f474887b9664fd7a171c17a | CC-BY 4.0 | glTF / FBX\* | Дешёвая фоновая машина |
| Полицейская машина | Hawkins Police Car | Sketchfab | https://sketchfab.com/3d-models/none-ff2a0433d7544fb6b1e23b719fe72e7f | CC-BY 4.0 | glTF / FBX\* | Периодная машина |
| Полицейская машина (Fab) | Police car | Fab | https://www.fab.com/listings/009451b7-15f4-4a95-a29b-cfbc55b23d89 | Free (EULA Fab) | См. страницу | Бесплатная машина с Fab |
| Машины | Police Car | Poly Pizza | https://poly.pizza/m/BwwnUrWGmV | CC0 | FBX / OBJ / glTF | Low-poly, Quaternius |
| Машины | Punk | Poly Pizza | https://poly.pizza/m/BTALZymknF | CC0 | FBX / OBJ / glTF | Фирменная машина группировки |
| Машины | Cars Pack | Quaternius | https://quaternius.com/packs/cars.html | CC0 | FBX / OBJ / Blend / glTF | Набор гражданского трафика |
| Машины | Car Kit | Kenney | https://kenney.nl/assets/car-kit | CC0 | FBX / OBJ / Blend / glTF | Модульные low-poly автомобили |
| Транспорт | Public Transport Pack | Quaternius | https://quaternius.com/packs/publictransport.html | CC0 | FBX / OBJ / Blend / glTF | Автобусы/такси для глубины улицы |
| Оружие | Baseball bat | Poly Haven | https://polyhaven.com/a/baseball_bat | CC0 | FBX / OBJ / Blend / glTF | Оружие фракции |
| Оружие | Baseball bat | Sketchfab | https://sketchfab.com/3d-models/none-2669cf86386747449611396c0409db47 | CC-BY 4.0 | glTF / FBX\* | Текстурированная альтернатива |
| Оружие | service_pistol | Poly Haven | https://polyhaven.com/a/service_pistol | CC0 | FBX / OBJ / Blend / glTF | Служебный пистолет детектива |
| Оружие | Pistol | Poly Pizza | https://poly.pizza/m/J3i9KDQ3kt | CC0 | FBX / OBJ / glTF | Low-poly альтернатива |
| Барьер | concrete_road_barrier | Poly Haven | https://polyhaven.com/a/concrete_road_barrier | CC0 | FBX / OBJ / Blend / glTF | Также `concrete_road_barrier_02` — оцепление полиции |

## 6. Текстуры и материалы

Все **CC0**, скачиваются наборами PNG/PBR 1K–4K (albedo, normal, roughness, AO, displacement).

| Категория | Название | Источник | URL | Лицензия | Формат | Примечания |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Мокрый асфальт | Asphalt025C | ambientCG | https://ambientcg.com/view?id=Asphalt025C | CC0 | PNG 1K–4K | Теги: *dark / rain / wet* — **главный** материал мокрой улицы |
| Асфальт | Asphalt033 | ambientCG | https://ambientcg.com/view?id=Asphalt033 | CC0 | PNG 1K–4K | Второй мокрый вариант для блендинга |
| Дорога | Road007 | ambientCG | https://ambientcg.com/view?id=Road007 | CC0 | PNG 1K–4K | Поверхность дороги с разметкой |
| Дорога | Road012A | ambientCG | https://ambientcg.com/view?id=Road012A | CC0 | PNG 1K–4K | Альтернативный тайл дороги |
| Дорога | Road013A | ambientCG | https://ambientcg.com/view?id=Road013A | CC0 | PNG 1K–4K | Альтернативный тайл дороги |
| Кирпич | Bricks097 | ambientCG | https://ambientcg.com/view?id=Bricks097 | CC0 | PNG 1K–4K | Стены переулков/зданий |
| Кирпич | Bricks060 | ambientCG | https://ambientcg.com/view?id=Bricks060 | CC0 | PNG 1K–4K | Старее, грязнее кирпич |
| Кирпич | Bricks089 | ambientCG | https://ambientcg.com/view?id=Bricks089 | CC0 | PNG 1K–4K | Третий кирпич для разнообразия |
| Паркет | WoodFloor064 | ambientCG | https://ambientcg.com/view?id=WoodFloor064 | CC0 | PNG 1K–4K | Тёмный паркет — пол бара |
| Паркет | WoodFloor043 | ambientCG | https://ambientcg.com/view?id=WoodFloor043 | CC0 | PNG 1K–4K | Альтернативное интерьерное дерево |
| Доски | Planks037A | ambientCG | https://ambientcg.com/view?id=Planks037A | CC0 | PNG 1K–4K | Дерево стойки/полок |
| Металл | Metal038 | ambientCG | https://ambientcg.com/view?id=Metal038 | CC0 | PNG 1K–4K | Шейкер, краны, перила |
| Ржавый металл | Metal056C | ambientCG | https://ambientcg.com/view?id=Metal056C | CC0 | PNG 1K–4K | Грязь для пожарных лестниц/дверей |
| Асфальт (альт.) | asphalt_02 | Poly Haven | https://polyhaven.com/a/asphalt_02 | CC0 | PNG / EXR 1K–8K | Дорога более высокого разрешения |
| Материалы (широко) | BlenderKit free materials | BlenderKit | https://www.blenderkit.com/asset-gallery/?query=category_subtree:material | Free tier (лицензия на ассет) | Blend | ⚠ Лицензия индивидуальна — фильтровать по *Free* и читать каждую страницу. Есть стекло, асфальт, кирпич |
| Материал стекла | Glass (free) | BlenderKit | https://www.blenderkit.com/asset-gallery/?query=category_subtree:glass | Free tier | Blend | База шейдера бутылок/бокалов |

> **Неон/emissive:** отдельный пак emissive-текстур не понадобился — неон строится прямо в UE из emissive-материала, который ведёт кривая (мерцание) поверх мешей вывесок из §2/§4. В Cyberpunk Game Kit от Quaternius уже идут меш-вывески в неоновых цветах.

## 7. Источники для UE5

| Категория | Название | Источник | URL | Лицензия | Формат | Примечания |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| VFX | Niagara Examples Pack (50+ систем) | Fab / Epic | https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600 | Free (EULA Fab) | `.uasset` (UE 5.7) | Дым, карточки тумана, искры, следы — покрывает дым бара и уличный туман |
| Набор бара | Stylized Low Poly Bar Counter Set | Fab | https://www.fab.com/listings/ec950ae1-4bf5-4957-9ce0-b220932a34c2 | Free (EULA Fab) | FBX / OBJ / Blend | Бесплатный набор бара прямой загрузки (также в §2) |
| Неон | CC0 - Neon Sign Open | Fab | https://www.fab.com/listings/4e687ce7-9aa0-43df-8868-c17da645bbeb | CC0 | См. страницу | Бесплатный меш вывески |
| Транспорт | Police car (SpatialNeglect) | Fab | https://www.fab.com/listings/009451b7-15f4-4a95-a29b-cfbc55b23d89 | Free (EULA Fab) | См. страницу | Бесплатная машина |
| Фотограмметрия | Quixel Megascans — Free library | Quixel / Fab | https://quixel.com/megascans/free | Free (EULA Fab) | `.uasset` / FBX + текстуры | Фотореалистичные поверхности, реквизит, декали для мокрой улицы |
| Поиск | Fab — фильтр бесплатно | Fab | https://www.fab.com/search?q=bar&price=free | Per-listing | Varies | ⚠ URL-параметр `price=free` у Fab работает нестабильно — всегда подтверждайте, что на странице стоит **Free** |
| Стартовый контент | UE Starter Content | Epic (ставится с UE) | https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-a-new-project-in-unreal-engine | Free (EULA UE) | `.uasset` | Создать проект → галочка *Starter Content*. Базовые материалы, реквизит, частицы; не нуар, но рабочая отправная точка |
| Маркетплейс | Ротация Fab «Free of the month» | Fab | https://www.fab.com/search?q=free | Бесплатно в момент выдачи | `.uasset` | ⚠ Раздачи с ограниченным сроком — **забирать в библиотеку, чтобы оставить**. Ротация примерно раз в месяц |

**Замечание о статусе Quixel/Fab:** quixel.com/megascans/free отвечает (200), но Megascans теперь отдаётся через Fab — на всё скачанное после миграции действует лицензия Fab, а не старая Quixel.

---

## 8. Чек-лист атрибуции и коммерческого использования

1. **Сначала CC0.** Poly Haven, ambientCG, Quaternius, Kenney и Poly Pizza не требуют ничего — начинайте с них для реквизита, текстур и уличных китов.
2. **CC-BY (Sketchfab) годится** для коммерческой игры, но кредитовать обязательно. Вести `docs/CREDITS.txt`: название ассета, автор, URL, лицензия, ссылка на текст лицензии.
3. **Избегайте CC-BY-SA** в релизном контенте, если есть CC-BY/CC0-эквивалент. Одна SA-позиция в списке — *Bar Shaker*; заменить на CC0-шейкер.
4. **Никогда** CC-BY-NC, NC-ND, только GPL и ничего, перезалитого из коммерческой игры. Всё такое при сканировании отбрасывалось (включая перезалив «Mafia II Definitive Edition»).
5. **Fab / Marketplace / Mixamo** живут по собственным EULA, а не Creative Commons: бесплатно использовать и включать в игру, но сырьё ассетами не перепродавать. Для Mixamo FBX вне проекта Epic перечитайте условия [FAQ Mixamo](https://www.mixamo.com).
6. **Проверяйте флаг *Downloadable*** на каждой странице Sketchfab — бесплатный просмотр ≠ бесплатная загрузка.
7. **Перепроверяйте в день загрузки.** Лицензию может поменять загрузчик; сохранять датированную копию текста лицензии на каждый ассет.

## 9. Рекомендуемый минимум (набор для грей-бокса игры)

| Потребность | Выбор | Почему |
| :--- | :--- | :--- |
| Помещение бара | *Pub interior* + *BarCounter - 8192px²* + *Wooden Back Bar Cabinet* | Один блокаут + два меша-героя |
| Стекло/бутылки | *The ultimate glass pack* + *Wine Bottles* (CC0) | Все типы заказов покрыты, без атрибуции |
| Посадка | *bar_chair_round_01*, *metal_stool_01*, *Bar Stool and Table* | Три силуэта, всё бесплатно |
| Персонажи | *Universal Base Characters* + клипы Mixamo | Ригнутые, ретаргетятся, анимаций без ограничений |
| Фракции | *Boss Mafia Man*, *Character (Type Detective)*, *Man In Coat* | Читаются на уровне силуэта |
| Экстерьер | *Cyberpunk Game Kit* + *Modular Streets Pack* + *Alleyway* | Совпадение по стилю + модульные дороги + задний переулок |
| Полиция | *Police Car* (Sketchfab) + *concrete_road_barrier* + *service_pistol* | Оцепление у бара |
| Материалы | *Asphalt025C* + *Bricks097* + *WoodFloor064* + *Planks037A* | Мокрая улица, стены переулка, пол/стойка |
| VFX | *Niagara Examples Pack* + бесплатный Quixel Megascans | Дым, туман, мокрые поверхности |

**Итоговая поверхность лицензий базового набора:** 4 позиции CC-BY для кредитования, всё остальное CC0 или EULA платформы.

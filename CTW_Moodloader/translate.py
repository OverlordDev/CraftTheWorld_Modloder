import os

replacements = {
    '[-] Ошибка загрузки скрипта ': '[-] Error loading script ',
    'успешно загружен.': 'successfully loaded.',
    'Скрипт': 'Script',
    '[SDK] Ручное добавление опыта: %d\\n': '[SDK] Manual exp addition: %d\\n',
    '[-] Ошибка: Объект CharLevels не найден! (Мир загружен?)\\n': '[-] Error: CharLevels object not found! (Is world loaded?)\\n',
    '[SDK] Вызываем окно уровня для объекта Game: 0x': '[SDK] Triggering level up dialog for Game: 0x',
    '[-] Ошибка: Game Instance еще не инициализирован!': '[-] Error: Game Instance not initialized yet!',
    '[SDK] Хук на OnEvent успешно активирован!': '[SDK] OnEvent hook successfully activated!',
    '[+] Хук Game::OnMouseClick активирован!': '[+] Game::OnMouseClick hook activated!',
    '[-] Ошибка: Не удалось поставить хук на Game::OnMouseClick!\\n': '[-] Error: Failed to hook Game::OnMouseClick!\\n',
    '[+] Хук World::OnMouseClick активирован!': '[+] World::OnMouseClick hook activated!',
    '[-] Ошибка: Не удалось поставить хук на World::OnMouseClick!\\n': '[-] Error: Failed to hook World::OnMouseClick!\\n',
    '[+] Хук Главного Меню (StartDialog::OnEvent) активен\\n': '[+] Main Menu hook (StartDialog::OnEvent) active\\n',
    '[+] Инициализация системы модов (modloader)\\n': '[+] Modloader system initialized\\n',
    '[+] Добавлено ресурса ID ': '[+] Added resource ID ',
    '--- НАЧИНАЕМ ГЛУБОКОЕ СКАНИРОВАНИЕ ---': '--- STARTING DEEP SCAN ---',
    '[BUILD] Блок %d установлен на [%d, %d]\\n': '[BUILD] Block %d placed at [%d, %d]\\n',
    '[+] MainWarehouseEntity::m_instance найден по адресу: 0x%X\\n': '[+] MainWarehouseEntity::m_instance found at: 0x%X\\n',
    '[-] Ошибка: MainWarehouseEntity::m_instance не найден или равен null!\\n': '[-] Error: MainWarehouseEntity::m_instance not found or null!\\n',
    '    (Мир загружен? Портал построен?)\\n': '    (Is world loaded? Portal built?)\\n',
    '[+] Гном #%d заспавнен через портал!\\n': '[+] Dwarf #%d spawned via portal!\\n',
    '[!] Портал занят, гном #%d ожидает...\\n': '[!] Portal busy, dwarf #%d waiting...\\n',
    '[-] Ошибка: MonstersManager не найден!': '[-] Error: MonstersManager not found!',
    'заспавнен на': 'spawned at',
    '[-] Ошибка: Не удалось заспавнить ': '[-] Error: Failed to spawn ',
    'на': 'at',
    'Овца заспавнена на': 'Sheep spawned at',
    'овцу на': 'sheep at',
    '[-] Ошибка ImGui: не удалось загрузить шрифт с кириллицей': '[-] ImGui Error: failed to load Cyrillic font',
    '[+] Консоль разработчика успешно загружена!': '[+] Developer console loaded successfully!',
    '[!] INSERT - Меню | F6 - Перезагрузить моды | END - Выход': '[!] INSERT - Menu | F6 - Reload Mods | END - Exit',
    '[!] Lua срипты успешно перезагружены (F6)\\n': '[!] Lua scripts successfully reloaded (F6)\\n',
    'Выгружаемся...': 'Unloading...'
}

files = ['LuaManager.cpp', 'GameAPI.cpp', 'dllmain.cpp']

for file in files:
    if os.path.exists(file):
        with open(file, 'r', encoding='utf-8') as f:
            content = f.read()
            
        for k, v in replacements.items():
            content = content.replace(k, v)
            
        with open(file, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Processed {file}")

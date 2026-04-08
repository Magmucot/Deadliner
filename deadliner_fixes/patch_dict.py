from __future__ import annotations

import re
from pathlib import Path

FILE_PATH = Path("src/ui/i18n/language_manager.cpp")
BEGIN_MARKER = "// BEGIN AUTO TRANSLATIONS"
END_MARKER = "// END AUTO TRANSLATIONS"

NEW_TRANSLATIONS: dict[str, str] = {
    "Tray icon": "Иконка в трее",
    "Variant 1": "Вариант 1",
    "Variant 2": "Вариант 2",
    "Overdue": "Просрочено",
    "Review one-time reminders that are already past due.": "Проверьте разовые напоминания, срок которых уже прошёл.",
    "No overdue reminders.": "Нет просроченных напоминаний.",
    "When a one-time reminder is missed, it will appear here instead of firing automatically.": "Когда разовое напоминание пропущено, оно появится здесь вместо автоматического срабатывания.",
    "Due at": "Срок",
    "Overdue by": "Просрочено на",
    "%1 d %2 h late": "Просрочено на %1 д %2 ч",
    "%1 h %2 min late": "Просрочено на %1 ч %2 мин",
    "%1 min late": "Просрочено на %1 мин",
}

ENTRY_RE = re.compile(r'\{"(?P<source>(?:[^"\\]|\\.)*)",\s*"(?P<translated>(?:[^"\\]|\\.)*)"\},')
BLOCK_RE = re.compile(
    rf"(?P<prefix>{re.escape(BEGIN_MARKER)}\n)(?P<body>.*?)(?P<suffix>\n\s*{re.escape(END_MARKER)})",
    re.DOTALL,
)


def unescape(value: str) -> str:
    return bytes(value, "utf-8").decode("unicode_escape")


def escape(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def load_block(text: str) -> tuple[str, str, str]:
    match = BLOCK_RE.search(text)
    if match is None:
        raise RuntimeError("translation block markers not found")
    return match.group("prefix"), match.group("body"), match.group("suffix")


def parse_entries(block: str) -> dict[str, str]:
    entries: dict[str, str] = {}
    for match in ENTRY_RE.finditer(block):
        entries[unescape(match.group("source"))] = unescape(match.group("translated"))
    return entries


def render_entries(entries: dict[str, str]) -> str:
    lines = []
    for source, translated in sorted(entries.items(), key=lambda item: item[0].lower()):
        lines.append(f'                {{"{escape(source)}", "{escape(translated)}"}},')
    return "\n".join(lines)


def main() -> None:
    text = FILE_PATH.read_text(encoding="utf-8")
    prefix, body, suffix = load_block(text)
    entries = parse_entries(body)
    entries.update(NEW_TRANSLATIONS)
    new_block = prefix + render_entries(entries) + suffix
    updated = BLOCK_RE.sub(new_block, text, count=1)
    FILE_PATH.write_text(updated, encoding="utf-8")
    print(f"Updated {FILE_PATH} with {len(NEW_TRANSLATIONS)} idempotent entries.")


if __name__ == "__main__":
    main()

# Time Master V4 — Changelog

## What's in V4

### Internationalisation (§ 0)
- New `core/I18n.{h,cpp}` singleton with two tables (English / 中文), 234 keys each.
- All UI strings route through `I18n::t("key")`; switch language from the sidebar pill or Settings.
- `categoryLabel()` / `priorityLabel()` in `Types.cpp` are language-aware.
- Persistent: language saved to QSettings (`language_mode`).

### Visual system (§ 2, § 3, § 6.5)
- Brand color darkened from `#D97757` → `#C26646` for WCAG AA contrast.
- Paper-warm palette: `#F2EEE5` page, `#FFFFFF` containers (light); `#26241F` page (dark).
- Removed page gradient & radial glow; flat `bgPage()` fill.
- Typography hierarchy: 22 / 15 / 14 / 13 / 12 px.
- KPI numbers: ~32 px DemiBold with tabular numerals (when Qt ≥ 6.7).
- Unified radii: cards 12 px, buttons/inputs 8 px, chips 6 px.

### Sidebar (§ 6.1)
- 232 → **180 px** wide. Logo 40 → 32 px. No English subtitle. No italic.
- Three ghost 28 × 28 bottom buttons: language toggle (`A` / `中`), theme, settings.

### Calendar page (§ 6.2, § 5.1, § 8)
- Single-row header: `[<] [>] Title [Today] … [Day|Week|Month] [+ New Event]`.
- **Ctrl + K** opens a Spotlight-style palette for AI parsing (replaces always-on input bar).
- Empty state with three starter templates (morning routine / deep work / weekly review).
- MonthView: weekend uses textSecondary, no today background fill (circle only), grid alpha 28 → 8, event bar 3 → 2 px.

### Analytics page (§ 5.2, § 6.3)
- Three section headers with brand accent bars: **Overview / Time structure / Behavioural insights**.
- EmptyState shown when no events exist in the past 7 days.
- Combined CategoryPieChart + HorizontalBarChart in one row.
- Source distribution renders as a horizontal stacked bar.

### Chat page (§ 4.2, § 5.3, § 6.4)
- AI persona rewritten — concise, professional, no `~呀啦哦人家` particles.
- Empty state with title + three clickable suggestion bubbles.
- **Privacy chip** replaces the orphan checkbox. Click to toggle "AI sees calendar".
- Persisted as `ai_sees_calendar` in QSettings.

### Onboarding (§ 5.4) — _added in this revision_
- First-run dialog with three steps: intro / language pick / API key.
- Triggered automatically when `onboarded` setting is absent.
- Tracks state via `QSettings("onboarded")`.

### Settings dialog
- New **Language** section (pill buttons English / 中文).
- Theme section uses the same pill button style.

### Quotes & copy (§ 4.1)
- Deleted all 15 `—— 时间管理大师` self-attribution quotes.
- Kept 15 quotes with real attributions (Buxton, Franklin, Lao Tzu, …).
- Removed italic on quote labels.

### Visual details (§ 7) — _added in this revision_
- Subtle drop shadow (`QGraphicsDropShadowEffect`) on stat cards and the calendar view card.
- Two strength levels — Subtle (calendar) and Card (stats); dark mode tones it down further.
- Single helper class `widgets/ShadowEffect.h` to keep all shadow profiles consistent.
- Skipped per spec: pseudo-glass blur (§ 7.1) — too costly for ~12 cards on dark mode.

### Removed copy
- `m_emptyHint` arrow widget ("← 从这里开始导入") on calendar page.
- Cute "小时" AI persona references.
- All self-attributed motivation quotes.

## What was NOT done
- § 7.1 pseudo-glass blur (perf risk on dark mode with many cards).
- § 7.3 hover **animations** beyond Qt stylesheet `:hover` color transitions (Qt stylesheets can't animate transforms/scale).

## Build
- Requires Qt **6.0+**; tabular-figure numerals on KPIs unlock at **6.7+**.
- `cmake -B build && cmake --build build`
- No new external dependencies.

const langMap = {
	"en-us": {
		name: "English",
		async strings() {
			return await import("../lang/en-us.json");
		},
	},
	"ru-ru": {
		name: "Русский",
		async strings() {
			return await import("../lang/ru-ru.json");
		},
	},
};

const rtlLanguages = new Set(["ar", "fa", "he"]);

/**
 * Map any saved or system locale onto the two supported UI languages.
 * @param {string} [code]
 * @returns {"en-us"|"ru-ru"}
 */
export function resolveUiLanguage(code) {
	const normalized = String(code || "")
		.toLowerCase()
		.replace(/_/g, "-");
	if (normalized === "ru" || normalized.startsWith("ru-")) return "ru-ru";
	return "en-us";
}

export function getIntlLocale(code) {
	const locale = resolveUiLanguage(code);

	try {
		return Intl.getCanonicalLocales(locale)[0];
	} catch {
		return "en-US";
	}
}

export function getLocaleDirection(locale) {
	const language = locale?.split("-")[0]?.toLowerCase();
	return rtlLanguages.has(language) ? "rtl" : "ltr";
}

export default {
	async set(code) {
		const lang = langMap[resolveUiLanguage(code)];
		const strings = await lang.strings();
		window.strings = strings.default;
	},
	list: Object.keys(langMap).map((code) => [code, langMap[code].name]),
	getName(code) {
		return langMap[resolveUiLanguage(code)].name;
	},
};

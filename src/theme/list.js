import fonts from "lib/fonts";
import settings from "lib/settings";
import { updateActiveTerminals } from "settings/terminalSettings";
import color from "utils/color";
import ThemeBuilder from "./builder";
import themes from "./preInstalled";

/** @type {Map<string, ThemeBuilder>} */
const appThemes = new Map();
let themeApplied = false;
let firstTime = true;

function init() {
	themes.forEach((theme) => add(theme));
}

/**
 * @typedef {object} Theme
 * @property {string} id
 * @property {string} name
 * @property {string} type
 * @property {string} version
 * @property {string} primaryColor
 */

/**
 * Returns a list of all themes
 * @returns {Theme[]}
 */
function list() {
	return Array.from(appThemes.keys()).map((name) => {
		const { id, type, primaryColor, version } = appThemes.get(name);
		return {
			id,
			type,
			version,
			primaryColor,
			name: name.capitalize(),
		};
	});
}

/**
 *
 * @param {string} name
 * @returns {ThemeBuilder}
 */
function get(name) {
	return appThemes.get(String(name || "").toLowerCase());
}

/**
 *
 * @param {ThemeBuilder} theme
 * @returns
 */
function add(theme) {
	if (!(theme instanceof ThemeBuilder)) return;
	if (appThemes.has(theme.id)) return;

	appThemes.set(theme.id, theme);

	if (theme.matches("dark") || theme.id === "dark") {
		apply("dark");
	}
}

/**
 * Apply a theme
 * @param {string} id The name of the theme to apply
 * @param {boolean} init Whether or not this is the first time the theme is being applied
 */
export async function apply(id, init) {
	id = "dark";

	themeApplied = true;
	const theme = get(id);
	if (!theme) return;

	const $style = document.head.get("style#app-theme") ?? (
		<style id="app-theme"></style>
	);
	const update = {
		appTheme: "dark",
		editorTheme: "one_dark",
	};

	if (init && editorManager != null && editorManager.editor != null) {
		editorManager.editor.setTheme("one_dark");
	}

	if (init && theme.preferredFont) {
		update.editorFont = theme.preferredFont;
		fonts.setFont(theme.preferredFont);
	}

	if (init && firstTime) {
		update.terminalSettings = {
			...(settings.value.terminalSettings || {}),
			theme: "dark",
		};
	}

	settings.update(update, false);

	if (init && firstTime) {
		if (editorManager != null) {
			updateActiveTerminals("theme", "dark");
		}
	}

	localStorage.__primary_color = theme.primaryColor;
	document.body.setAttribute("theme-type", theme.type);
	$style.textContent = theme.css;
	document.head.append($style);

	const primaryColor = color(theme.primaryColor).hex.toString();
	const scheme = theme.toJSON("hex");
	system.setUiTheme(primaryColor, scheme);

	if (firstTime) {
		setTimeout(() => {
			system.setUiTheme(primaryColor, scheme);
		}, 1000);
		firstTime = false;
	}
}

/**
 * Update a theme
 * @param {ThemeBuilder} theme
 */
export function update(theme) {
	if (!(theme instanceof ThemeBuilder)) return;
	const oldTheme = get(theme.id);
	if (!oldTheme) {
		add(theme);
		return;
	}
	const json = theme.toJSON();
	Object.keys(json).forEach((key) => {
		oldTheme[key] = json[key];
	});
}

export function updateSystemThemeWatcher() {}

export default {
	get applied() {
		return themeApplied;
	},
	init,
	list,
	get,
	add,
	apply,
	update,
	updateSystemThemeWatcher,
};

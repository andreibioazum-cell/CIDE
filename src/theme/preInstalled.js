import { createBuiltInTheme } from "./builder";

export function getSystemEditorTheme() {
	return "one_dark";
}

export function updateSystemTheme() {}

const dark = createBuiltInTheme("Dark", "dark", "free");
dark.primaryColor = "rgb(35, 39, 42)";
dark.primaryTextColor = "rgb(245, 245, 245)";
dark.secondaryColor = "rgb(45, 49, 52)";
dark.secondaryTextColor = "rgb(228, 228, 228)";
dark.activeColor = "rgb(66, 133, 244)";
dark.linkTextColor = "rgb(138, 180, 248)";
dark.borderColor = "rgba(188, 188, 188, 0.15)";
dark.popupIconColor = "rgb(245, 245, 245)";
dark.popupBackgroundColor = "rgb(35, 39, 42)";
dark.popupTextColor = "rgb(245, 245, 245)";
dark.popupActiveColor = "rgb(66, 133, 244)";
dark.activeTextColor = "rgb(255, 255, 255)";
dark.errorTextColor = "rgb(255, 185, 92)";
dark.dangerColor = "rgb(220, 38, 38)";
dark.scrollbarColor = "rgba(255, 255, 255, 0.2)";
dark.preferredEditorTheme = "one_dark";
dark.preferredTerminalTheme = "dark";

export default [dark];

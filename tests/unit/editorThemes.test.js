import { EditorState } from "@codemirror/state";
import { describe, expect, it } from "vitest";
import {
	getThemeConfig,
	getThemeExtensions,
	getThemes,
} from "cm/themes";

describe("built-in editor themes", () => {
	it("registers One Dark as the only built-in editor theme", () => {
		expect(getThemes().map((theme) => theme.id)).toEqual(["one_dark"]);
		expect(getThemeConfig("one_dark")).toMatchObject({
			background: "#282c34",
			foreground: "#abb2bf",
		});
	});

	it("constructs every registered editor theme", () => {
		for (const { id } of getThemes()) {
			expect(() =>
				EditorState.create({ extensions: getThemeExtensions(id) }),
			).not.toThrow();
		}
	});
});

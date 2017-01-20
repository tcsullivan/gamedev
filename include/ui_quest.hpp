/**
 * @file ui_quest.hpp
 * @brief Handles UI elements related to quests.
 */
#ifndef UI_QUEST_HPP_
#define UI_QUEST_HPP_

#include <ui.hpp>
#include <vector2.hpp>

extern vec2 offset;

namespace ui {
	namespace quest {
		/**
		 * A flag to determine if the UI should be drawn.
		 */
		bool _toggle = false;

		/**
		 * Toggles displaying of the UI.
		 */
		inline void toggle(void)
		{ _toggle ^= true; }

		/**
		 * Draws the quest UI to the screen, if enabled.
		 */
		void draw(void) {
			static unsigned int textWrap = 40;

			if (!_toggle)
				return;

			std::swap(textWrap, ui::textWrapLimit);

			float top_y = offset.y + 200;
			ui::drawNiceBox(vec2 {offset.x - 200, top_y },
			                vec2 {offset.x + 200, offset.y - 200 },
			                -0.7f);

			ui::putStringCentered(offset.x, top_y - 40, "Current Quests:");
			
			/*auto y = top_y - 100;
			const auto x = offset.x - 180;
			for (const auto &q : player->qh.current) {
				ui::putText(x, y, q.title.c_str());
				y -= 20;
				ui::putText(x + 40, y, q.desc.c_str());
				y -= 40; 
			}*/

			std::swap(textWrap, ui::textWrapLimit);
		}
	}
}

#endif // UI_QUEST_HPP_

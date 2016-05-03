#include <ui_action.hpp>
#include <world.hpp>

#define ACTION_PROLOUGE { actioning = true; }
#define ACTION_EPILOUGE { actioning = false; actionHover = 0; }

extern World *currentWorld;
extern Arena *arena;
extern Player *player;
extern bool inBattle;

static std::vector<std::pair<std::string, vec3>> actionText = {
    {"Attack", vec3 {0, 0, 0}},
    {"Action", vec3 {0, 0, 0}},
    {"Umm"   , vec3 {0, 0, 0}}
};

void actionAttack(void);
void actionAction(void);

static std::vector<void (*)(void)> actionFunc = {
    actionAttack,
    actionAction,
    nullptr,
};

static bool actionToggle = false, actioning = false;
static unsigned int actionHover = 0;
static Entity *nearEntity = nullptr, *lastEntity = nullptr;

namespace ui {
    namespace action {
        bool make = false;

        // enables action ui
        void enable(void) {
            actionToggle = true;
        }

        // disables action ui
        void disable(void) {
            actionToggle = false;

            if (lastEntity != nullptr)
                lastEntity->canMove = true;
        }

        // draws the action ui
        void draw(vec2 loc) {
            unsigned int i = 1;
            float y = loc.y;

            if (!actionToggle)
                return;

            nearEntity = currentWorld->getNearInteractable(*player);

            if (nearEntity == nullptr) {
                if (lastEntity != nullptr) {
                    lastEntity->canMove = true;
                    lastEntity = nullptr;
                }
                return;
            } else if (nearEntity != lastEntity) {
                if (lastEntity != nullptr)
                    lastEntity->canMove = true;
                lastEntity = nearEntity;
            }

            if (make) {
                while(actioning);

                if (!actionHover) {
                    make = false;
                    return;
                }

                if (actionFunc[actionHover - 1] != nullptr)
                    std::thread(actionFunc[actionHover - 1]).detach();

                actionHover = 0;
            } else {
                nearEntity->canMove = false;
                ui::drawBox(vec2 {loc.x - HLINES(11), loc.y}, vec2 {loc.x + HLINES(12), loc.y + actionText.size() * HLINES(8)});

                for (auto &s : actionText) {
                    s.second.z = ui::putStringCentered((s.second.x = loc.x), (s.second.y = (y += fontSize * 1.15f)), s.first) / 2;

                    if (ui::mouse.x > s.second.x - s.second.z && ui::mouse.x < s.second.x + s.second.z &&
                        ui::mouse.y > s.second.y && ui::mouse.y < s.second.y + ui::fontSize) {
                        actionHover = i;
                        ui::setFontColor(255, 100, 100, 255);
                        ui::putStringCentered(s.second.x, s.second.y, s.first);
                        ui::setFontColor(255, 255, 255, 255);
                    }
                    i++;
                }

                ui::putStringCentered(loc.x, y + fontSize * 1.2f, nearEntity->name);
            }

            if (i == actionText.size())
                actionHover = 0;

            ui::setFontColor(255, 255, 255, 255);
            make = false;
        }
    }
}

void actionAttack(void)
{
    ACTION_PROLOUGE;

    auto m = currentWorld->getNearInteractable(*player);

    if (m->type == MOBT) {
        if (!inBattle && m != nullptr) {
            arena->fight(currentWorld, player, Mobp(m));

            ui::toggleWhiteFast();
            ui::waitForCover();
            currentWorld = arena;
            ui::toggleWhiteFast();
        }
    } else {
        ui::dialogBox(player->name, "", false, "%s doesn't appear to be in the mood for fighting...", m->name);
    }

    ACTION_EPILOUGE;
}

void actionAction(void)
{
    ACTION_PROLOUGE;

    auto e = currentWorld->getNearInteractable(*player);

    if (e->type == NPCT)
        e->interact();

    ACTION_EPILOUGE;
}

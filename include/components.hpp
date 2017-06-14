/**
 * @file components.hpp
 * @brief Where all of an enities possible components are stored.
 * Using an ECS (Entity component system) the entities are given components on the fly,
 * this allows the entity to change stats or skills on the go. This also allows every "object"
 * the be an entity, and it gives the game a much better customizability over xml.
 */
#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include <components/all.hpp>

#include <systems/dialog.hpp>
#include <systems/movement.hpp>
#include <systems/physics.hpp>
#include <systems/render.hpp>

#endif //COMPONENTS_HPP

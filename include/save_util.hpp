#ifndef SAVE_UTIL_H_
#define SAVE_UTIL_H_

/*
 * Save macros.
 */

#define E_SAVE_COORDS { xmle->SetAttribute("x", loc.x); xmle->SetAttribute("y", loc.y); }

#define E_SAVE_HEALTH xmle->SetAttribute("health", health);

/*
 * Load macos.
 */

#define E_LOAD_COORDS(yy) { float n; \
						  if (xmle->QueryFloatAttribute("x", &n) == XML_NO_ERROR) \
							  spawn(n, yy); \
						  else \
							  spawn(xmle->FloatAttribute("spawnx"), 100); \
						  \
						  if (xmle->QueryFloatAttribute("y", &n) == XML_NO_ERROR) \
							  loc.y = n; }

#define E_LOAD_HEALTH   { float n; \
						  \
						  if (xmle->QueryFloatAttribute("maxHealth", &n) != XML_NO_ERROR) \
							  maxHealth = 1; \
						  \
						  if (xmle->QueryFloatAttribute("health", &n) == XML_NO_ERROR) \
							  health = n; \
						  else \
							  health = maxHealth; }


#endif // SAVE_UTIL_H_

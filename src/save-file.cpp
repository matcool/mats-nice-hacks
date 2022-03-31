#include "save-file.hpp"
#include <matdash.hpp>
#include <matdash/minhook.hpp>
#include "gd.hpp"
#include "utils.hpp"

using namespace matdash;


void EditorPauseLayer_saveLevel(EditorPauseLayer* self) {
	println("save level called! ???");
}

void save_file::init() {
	// add_hook<&EditorPauseLayer_saveLevel>(base + 0x3eec0);

}
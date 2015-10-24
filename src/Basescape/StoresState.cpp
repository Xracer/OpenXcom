/*
 * Copyright 2010-2015 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "StoresState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Base.h"
#include "../Mod/RuleItem.h"
#include "../Savegame/ItemContainer.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Stores window.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 */
StoresState::StoresState(Base *base) : _base(base)
{
	// Create objects
	_window = new Window(this, 400, 200, 500, 400, POPUP_BOTH);
	_btnOk = new TextButton(300, 16, 510, 576);
	_txtTitle = new Text(310, 17, 505, 408);
	_txtItem = new Text(142, 9, 510, 432);
	_txtQuantity = new Text(88, 9, 652, 432);
	_txtSpaceUsed = new Text(74, 9, 740, 432);
	_lstStores = new TextList(288, 128, 508, 440);

	// Set palette
	setInterface("storesInfo");

	add(_window, "window", "storesInfo");
	add(_btnOk, "button", "storesInfo");
	add(_txtTitle, "text", "storesInfo");
	add(_txtItem, "text", "storesInfo");
	add(_txtQuantity, "text", "storesInfo");
	add(_txtSpaceUsed, "text", "storesInfo");
	add(_lstStores, "list", "storesInfo");

	//centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface("BACK13.SCR"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&StoresState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&StoresState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&StoresState::btnOkClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_STORES"));

	_txtItem->setText(tr("STR_ITEM"));

	_txtQuantity->setText(tr("STR_QUANTITY_UC"));

	_txtSpaceUsed->setText(tr("STR_SPACE_USED_UC"));

	_lstStores->setColumns(3, 162, 92, 32);
	_lstStores->setSelectable(true);
	_lstStores->setBackground(_window);
	_lstStores->setMargin(2);

	const std::vector<std::string> &items = _game->getMod()->getItemsList();
	for (std::vector<std::string>::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		int qty = _base->getStorageItems()->getItem(*i);
		if (qty > 0)
		{
			RuleItem *rule = _game->getMod()->getItem(*i);
			std::wostringstream ss, ss2;
			ss << qty;
			ss2 << qty * rule->getSize();
			_lstStores->addRow(3, tr(*i).c_str(), ss.str().c_str(), ss2.str().c_str());
		}
	}
}

/**
 *
 */
StoresState::~StoresState()
{

}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void StoresState::btnOkClick(Action *)
{
	_game->popState();
}

}

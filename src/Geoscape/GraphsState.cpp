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
#include "GraphsState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Engine/Palette.h"
#include "../Engine/Surface.h"
#include "../Engine/Screen.h"
#include "../Engine/InteractiveSurface.h"
#include "../Savegame/Country.h"
#include "../Savegame/Region.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleRegion.h"
#include "../Interface/Frame.h"
#include "../Interface/Text.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"
#include "../Savegame/GameTime.h"
#include "../Savegame/SavedGame.h"
#include "../Interface/TextList.h"
#include "../Engine/Action.h"
#include "../Engine/Options.h"
#include "../Mod/RuleInterface.h"

namespace OpenXcom
{

	struct GraphButInfo
	{
		LocalizedText _name;
		int _color;
		bool _pushed;
		GraphButInfo(const LocalizedText& name, Uint8 color) : _name(name), _color(color), _pushed(false) {}
	};
	/**
	* Initializes all the elements in the Graphs screen.
	* @param game Pointer to the core game.
	*/
	GraphsState::GraphsState() : _butRegionsOffset(0), _butCountriesOffset(0)
	{
		_screen = false;

		// Create objects
		_window = new Window(this, 640, 360, 160, 100, POPUP_NONE);
		_bg = new InteractiveSurface(640, 400, 475, 105);
		_bg->onMousePress((ActionHandler)&GraphsState::shiftButtons, SDL_BUTTON_WHEELUP);
		_bg->onMousePress((ActionHandler)&GraphsState::shiftButtons, SDL_BUTTON_WHEELDOWN);
		_grid = new InteractiveSurface(420, 225, 320, 195);
		_btnUfoRegion = new InteractiveSurface(32, 24, 571, 105);
		_btnUfoCountry = new InteractiveSurface(32, 24, 603, 105);
		_btnXcomRegion = new InteractiveSurface(32, 24, 635, 105);
		_btnXcomCountry = new InteractiveSurface(32, 24, 667, 105);
		_btnIncome = new InteractiveSurface(32, 24, 699, 105);
		_btnFinance = new InteractiveSurface(32, 24, 731, 105);
		_btnGeoscape = new InteractiveSurface(32, 24, 763, 105);
		_txtTitle = new Text(500, 16, 270, 140);
		_txtFactor = new Text(55, 11, 300, 170); // the isolated text above the columns value
		_txtMonths = new TextList(450, 11, 321, 420);
		_txtYears = new TextList(450, 11, 321, 436);

		// Set palette
		setInterface("graphs");

		//add all our elements
		add(_window, "window", "graphs");
		add(_bg);
		add(_grid);
		add(_btnUfoRegion);
		add(_btnUfoCountry);
		add(_btnXcomRegion);
		add(_btnXcomCountry);
		add(_btnIncome);
		add(_btnFinance);
		add(_btnGeoscape);
		add(_txtMonths, "scale", "graphs");
		add(_txtYears, "scale", "graphs");
		add(_txtTitle, "text", "graphs");
		add(_txtFactor, "text", "graphs");
		for (int scaleText = 0; scaleText != 10; ++scaleText)
		{
			_txtScale.push_back(new Text(55, 16, 264, 414 - (scaleText * 25))); // The location of the scale of the grid (scaleText*14) -> (scaleText*25)
			add(_txtScale.at(scaleText), "scale", "graphs");
		}
		Uint8 regionTotalColor = _game->getMod()->getInterface("graphs")->getElement("regionTotal")->color;
		Uint8 countryTotalColor = _game->getMod()->getInterface("graphs")->getElement("countryTotal")->color;

		//create buttons (sooooo many buttons)
		size_t offset = 0;
		for (std::vector<Region *>::iterator iter = _game->getSavedGame()->getRegions()->begin(); iter != _game->getSavedGame()->getRegions()->end(); ++iter)
		{
			// always save in toggles all the regions
			_regionToggles.push_back(new GraphButInfo(tr((*iter)->getRules()->getType()), 13 + (8 * offset)));
			// initially add the GRAPH_MAX_BUTTONS having the first regions information
			if (offset < GRAPH_MAX_BUTTONS)
			{
				_btnRegions.push_back(new ToggleTextButton(105, 16, 165, 105 + offset * 18)); //controls size location of icon list buttons of the countries,  offset * 18 -> 105 + (offset * 18) controls the location of the icon list y
				_btnRegions.at(offset)->setText(tr((*iter)->getRules()->getType()));
				_btnRegions.at(offset)->setInvertColor(13 + (8 * offset));
				_btnRegions.at(offset)->onMousePress((ActionHandler)&GraphsState::btnRegionListClick);
				_btnRegions.at(offset)->onMousePress((ActionHandler)&GraphsState::shiftButtons, SDL_BUTTON_WHEELUP);
				_btnRegions.at(offset)->onMousePress((ActionHandler)&GraphsState::shiftButtons, SDL_BUTTON_WHEELDOWN);
				add(_btnRegions.at(offset), "button", "graphs");
			}
			_alienRegionLines.push_back(new Surface(640, 400, 165, 105));
			add(_alienRegionLines.at(offset));
			_xcomRegionLines.push_back(new Surface(640, 400, 165, 105));
			add(_xcomRegionLines.at(offset));

			++offset;
		}

		if (_regionToggles.size() < GRAPH_MAX_BUTTONS)
			_btnRegionTotal = new ToggleTextButton(105, 16, 165, 105 + _regionToggles.size() * 18); // This controls the Total button _regionToggles.size() * 18);
		else
			_btnRegionTotal = new ToggleTextButton(105, 16, 165, 105 + GRAPH_MAX_BUTTONS * 18); // This controls the Total button GRAPH_MAX_BUTTONS * 18);
		_regionToggles.push_back(new GraphButInfo(tr("STR_TOTAL_UC"), regionTotalColor));
		_btnRegionTotal->onMousePress((ActionHandler)&GraphsState::btnRegionListClick);
		_btnRegionTotal->setInvertColor(regionTotalColor);
		_btnRegionTotal->setText(tr("STR_TOTAL_UC"));
		_alienRegionLines.push_back(new Surface(640, 400, 165, 105));
		add(_alienRegionLines.at(offset));
		_xcomRegionLines.push_back(new Surface(640, 400, 165, 105));
		add(_xcomRegionLines.at(offset));
		add(_btnRegionTotal, "button", "graphs");

		offset = 0;
		for (std::vector<Country *>::iterator iter = _game->getSavedGame()->getCountries()->begin(); iter != _game->getSavedGame()->getCountries()->end(); ++iter)
		{
			// always save in toggles all the countries
			_countryToggles.push_back(new GraphButInfo(tr((*iter)->getRules()->getType()), 13 + (8 * offset)));
			// initially add the GRAPH_MAX_BUTTONS having the first countries information
			if (offset < GRAPH_MAX_BUTTONS)
			{
				_btnCountries.push_back(new ToggleTextButton(105, 16, 165, 105 + offset * 18)); //controls size location of icon list buttons of the regions
				_btnCountries.at(offset)->setInvertColor(13 + (8 * offset));
				_btnCountries.at(offset)->setText(tr((*iter)->getRules()->getType()));
				_btnCountries.at(offset)->onMousePress((ActionHandler)&GraphsState::btnCountryListClick);
				_btnCountries.at(offset)->onMousePress((ActionHandler)&GraphsState::shiftButtons, SDL_BUTTON_WHEELUP);
				_btnCountries.at(offset)->onMousePress((ActionHandler)&GraphsState::shiftButtons, SDL_BUTTON_WHEELDOWN);
				add(_btnCountries.at(offset), "button", "graphs");
			}
			_alienCountryLines.push_back(new Surface(640, 400, 165, 105));
			add(_alienCountryLines.at(offset));
			_xcomCountryLines.push_back(new Surface(640, 400, 165, 105));
			add(_xcomCountryLines.at(offset));
			_incomeLines.push_back(new Surface(640, 400, 165, 105));
			add(_incomeLines.at(offset));

			++offset;
		}

		if (_countryToggles.size() < GRAPH_MAX_BUTTONS)
			_btnCountryTotal = new ToggleTextButton(105, 16, 165, 105 + _countryToggles.size() * 18);
		else
			_btnCountryTotal = new ToggleTextButton(105, 16, 165, 105 + GRAPH_MAX_BUTTONS * 18);
		_countryToggles.push_back(new GraphButInfo(tr("STR_TOTAL_UC"), countryTotalColor));
		_btnCountryTotal->onMousePress((ActionHandler)&GraphsState::btnCountryListClick);
		_btnCountryTotal->setInvertColor(countryTotalColor);
		_btnCountryTotal->setText(tr("STR_TOTAL_UC"));
		_alienCountryLines.push_back(new Surface(640, 400, 165, 105));
		add(_alienCountryLines.at(offset));
		_xcomCountryLines.push_back(new Surface(640, 400, 165, 105));
		add(_xcomCountryLines.at(offset));
		_incomeLines.push_back(new Surface(640, 400, 165, 105));
		add(_incomeLines.at(offset));
		add(_btnCountryTotal, "button", "graphs");


		for (int iter = 0; iter != 5; ++iter)
		{
			offset = iter;
			_btnFinances.push_back(new ToggleTextButton(105, 16, 165, 105 + offset * 18));
			_financeToggles.push_back(false);
			_btnFinances.at(offset)->setInvertColor(16 + (8 * offset));
			_btnFinances.at(offset)->onMousePress((ActionHandler)&GraphsState::btnFinanceListClick);
			add(_btnFinances.at(offset), "button", "graphs");
			_financeLines.push_back(new Surface(640, 400, 165, 105));
			add(_financeLines.at(offset));
		}

		_btnFinances.at(0)->setText(tr("STR_INCOME"));
		_btnFinances.at(1)->setText(tr("STR_EXPENDITURE"));
		_btnFinances.at(2)->setText(tr("STR_MAINTENANCE"));
		_btnFinances.at(3)->setText(tr("STR_BALANCE"));
		_btnFinances.at(4)->setText(tr("STR_SCORE"));

		// load back the button states
		std::string graphRegionToggles = _game->getSavedGame()->getGraphRegionToggles();
		std::string graphCountryToggles = _game->getSavedGame()->getGraphCountryToggles();
		std::string graphFinanceToggles = _game->getSavedGame()->getGraphFinanceToggles();
		while (graphRegionToggles.size() < _regionToggles.size()) graphRegionToggles.push_back('0');
		while (graphCountryToggles.size() < _countryToggles.size()) graphCountryToggles.push_back('0');
		while (graphFinanceToggles.size() < _financeToggles.size()) graphFinanceToggles.push_back('0');
		for (size_t i = 0; i < _regionToggles.size(); ++i)
		{
			_regionToggles[i]->_pushed = ('0' == graphRegionToggles[i]) ? false : true;
			if (_regionToggles.size() - 1 == i)
				_btnRegionTotal->setPressed(_regionToggles[i]->_pushed);
			else if (i < GRAPH_MAX_BUTTONS)
				_btnRegions.at(i)->setPressed(_regionToggles[i]->_pushed);
		}
		for (size_t i = 0; i < _countryToggles.size(); ++i)
		{
			_countryToggles[i]->_pushed = ('0' == graphCountryToggles[i]) ? false : true;
			if (_countryToggles.size() - 1 == i)
				_btnCountryTotal->setPressed(_countryToggles[i]->_pushed);
			else if (i < GRAPH_MAX_BUTTONS)
				_btnCountries.at(i)->setPressed(_countryToggles[i]->_pushed);
		}
		for (size_t i = 0; i < _financeToggles.size(); ++i)
		{
			_financeToggles[i] = ('0' == graphFinanceToggles[i]) ? false : true;
			_btnFinances.at(i)->setPressed(_financeToggles[i]);
		}
		Uint8 gridColor = _game->getMod()->getInterface("graphs")->getElement("graph")->color;
		// set up the grid
		_grid->drawRect(160, 95, 0, 250, gridColor); //change (125, 49, 162, 162) to (160, 95, 400, 250) x,y is location of bg x',y' is size of bg

		for (int grid = 0; grid != 5; ++grid)
		{
			for (int y = 0 + grid; y <= 225 + grid; y += 25) /// y=50 -> 100, y'=160 -> 325, y''=14 -> 25, y is location of grid y' is height is grid y'' is cell height
			{
				for (int x = 0 + grid; x <= 420 + grid; x += 35) //// 17 changed to 35 x = 126 -> 200; x is location of grid because the bg is linked to teh grid
				{
					Uint8 color = gridColor + grid + 1;
					if (grid == 4)
					{
						color = 0;
					}
					_grid->drawRect(x, y, 35 - (grid * 2), 25 - (grid * 2), color); ///35 <- 16 - (grid*2), 25 <- 13 - (grid*2)
				}
			}
		}

		//set up the horizontal measurement units
		std::string months[] = { "STR_JAN", "STR_FEB", "STR_MAR", "STR_APR", "STR_MAY", "STR_JUN", "STR_JUL", "STR_AUG", "STR_SEP", "STR_OCT", "STR_NOV", "STR_DEC" };
		int month = _game->getSavedGame()->getTime()->getMonth();
		// i know using textlist for this is ugly and brutal, but YOU try getting this damn text to line up.
		// also, there's nothing wrong with being ugly or brutal, you should learn tolerance.
		_txtMonths->setColumns(12, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35);
		_txtMonths->addRow(12, L" ", L" ", L" ", L" ", L" ", L" ", L" ", L" ", L" ", L" ", L" ", L" ");
		_txtYears->setColumns(6, 70, 70, 70, 70, 70, 70);
		_txtYears->addRow(6, L" ", L" ", L" ", L" ", L" ", L" ");

		for (int iter = 0; iter != 12; ++iter)
		{
			if (month > 11)
			{
				month = 0;
				std::wostringstream ss;
				ss << _game->getSavedGame()->getTime()->getYear();
				_txtYears->setCellText(0, iter / 2, ss.str());
				if (iter > 2)
				{
					std::wostringstream ss2;
					ss2 << (_game->getSavedGame()->getTime()->getYear() - 1);
					_txtYears->setCellText(0, 0, ss2.str());
				}
			}
			_txtMonths->setCellText(0, iter, tr(months[month]));
			++month;
		}

		// set up the vertical measurement units
		for (std::vector<Text *>::iterator iter = _txtScale.begin(); iter != _txtScale.end(); ++iter)
		{
			(*iter)->setAlign(ALIGN_RIGHT);
		}
		btnUfoRegionClick(0);

		// Set up objects
		_window->setColor(154); //	This Black looks decent, now to move the lines :S
		_window->setThinBorder();

		if (_game->getMod()->getSurface("GRAPH.BDY"))
		{
			_game->getMod()->getSurface("GRAPH.BDY")->blit(_bg);
		}
		else
		{
			_game->getMod()->getSurface("GRAPHS.SPK")->blit(_bg);
		}

		_txtTitle->setAlign(ALIGN_CENTER);

		_txtFactor->setText(L"$1000's");

		// Set up buttons
		_btnUfoRegion->onMousePress((ActionHandler)&GraphsState::btnUfoRegionClick);
		_btnUfoCountry->onMousePress((ActionHandler)&GraphsState::btnUfoCountryClick);
		_btnXcomRegion->onMousePress((ActionHandler)&GraphsState::btnXcomRegionClick);
		_btnXcomCountry->onMousePress((ActionHandler)&GraphsState::btnXcomCountryClick);
		_btnIncome->onMousePress((ActionHandler)&GraphsState::btnIncomeClick);
		_btnFinance->onMousePress((ActionHandler)&GraphsState::btnFinanceClick);
		_btnGeoscape->onMousePress((ActionHandler)&GraphsState::btnGeoscapeClick);
		_btnGeoscape->onKeyboardPress((ActionHandler)&GraphsState::btnGeoscapeClick, Options::keyCancel);
		_btnGeoscape->onKeyboardPress((ActionHandler)&GraphsState::btnGeoscapeClick, Options::keyGeoGraphs);

		centerAllSurfaces();
	}

	/**
	*
	*/
	GraphsState::~GraphsState()
	{
		std::string graphRegionToggles;
		std::string graphCountryToggles;
		std::string graphFinanceToggles;
		for (size_t i = 0; i < _regionToggles.size(); ++i)
		{
			graphRegionToggles.push_back(_regionToggles[i]->_pushed ? '1' : '0');
			delete _regionToggles[i];
		}
		for (size_t i = 0; i < _countryToggles.size(); ++i)
		{
			graphCountryToggles.push_back(_countryToggles[i]->_pushed ? '1' : '0');
			delete _countryToggles[i];
		}
		for (size_t i = 0; i < _financeToggles.size(); ++i)
		{
			graphFinanceToggles.push_back(_financeToggles[i] ? '1' : '0');
		}
		_game->getSavedGame()->setGraphRegionToggles(graphRegionToggles);
		_game->getSavedGame()->setGraphCountryToggles(graphCountryToggles);
		_game->getSavedGame()->setGraphFinanceToggles(graphFinanceToggles);
	}

	/**
	* Returns to the previous screen.
	* @param action Pointer to an action.
	*/
	void GraphsState::btnGeoscapeClick(Action *)
	{
		_game->popState();
	}

	/**
	* Switches to the UFO Region Activity screen.
	* @param action Pointer to an action.
	*/
	void GraphsState::btnUfoRegionClick(Action *)
	{
		_alien = true;
		_income = false;
		_country = false;
		_finance = false;
		resetScreen();
		drawLines();
		for (std::vector<ToggleTextButton *>::iterator iter = _btnRegions.begin(); iter != _btnRegions.end(); ++iter)
		{
			(*iter)->setVisible(true);
		}
		_btnRegionTotal->setVisible(true);
		_txtTitle->setBig();
		_txtTitle->setText(tr("STR_UFO_ACTIVITY_IN_AREAS"));
	}

	/**
	* Switches to the UFO Country activity screen.
	* @param action Pointer to an action.
	*/
	void GraphsState::btnUfoCountryClick(Action *)
	{
		_alien = true;
		_income = false;
		_country = true;
		_finance = false;
		resetScreen();
		drawLines();
		for (std::vector<ToggleTextButton *>::iterator iter = _btnCountries.begin(); iter != _btnCountries.end(); ++iter)
		{
			(*iter)->setVisible(true);
		}
		_btnCountryTotal->setVisible(true);
		_txtTitle->setBig();
		_txtTitle->setText(tr("STR_UFO_ACTIVITY_IN_COUNTRIES"));
	}

	/**
	* Switches to the XCom Region activity screen.
	* @param action Pointer to an action.
	*/
	void GraphsState::btnXcomRegionClick(Action *)
	{
		_alien = false;
		_income = false;
		_country = false;
		_finance = false;
		resetScreen();
		drawLines();
		for (std::vector<ToggleTextButton *>::iterator iter = _btnRegions.begin(); iter != _btnRegions.end(); ++iter)
		{
			(*iter)->setVisible(true);
		}
		_btnRegionTotal->setVisible(true);
		_txtTitle->setBig();
		_txtTitle->setText(tr("STR_XCOM_ACTIVITY_IN_AREAS"));
	}

	/**
	* Switches to the XCom Country activity screen.
	* @param action Pointer to an action.
	*/
	void GraphsState::btnXcomCountryClick(Action *)
	{
		_alien = false;
		_income = false;
		_country = true;
		_finance = false;
		resetScreen();
		drawLines();
		for (std::vector<ToggleTextButton *>::iterator iter = _btnCountries.begin(); iter != _btnCountries.end(); ++iter)
		{
			(*iter)->setVisible(true);
		}
		_btnCountryTotal->setVisible(true);
		_txtTitle->setBig();
		_txtTitle->setText(tr("STR_XCOM_ACTIVITY_IN_COUNTRIES"));
	}

	/**
	* Switches to the Income screen.
	* @param action Pointer to an action.
	*/
	void GraphsState::btnIncomeClick(Action *)
	{
		_alien = false;
		_income = true;
		_country = true;
		_finance = false;
		resetScreen();
		drawLines();
		_txtFactor->setVisible(true);
		for (std::vector<ToggleTextButton *>::iterator iter = _btnCountries.begin(); iter != _btnCountries.end(); ++iter)
		{
			(*iter)->setVisible(true);
		}
		_btnCountryTotal->setVisible(true);
		_txtTitle->setBig();
		_txtTitle->setText(tr("STR_INCOME"));
	}

	/**
	* Switches to the Finances screen.
	* @param action Pointer to an action.
	*/
	void GraphsState::btnFinanceClick(Action *)
	{
		_alien = false;
		_income = false;
		_country = false;
		_finance = true;
		resetScreen();
		drawLines();

		for (std::vector<ToggleTextButton *>::iterator iter = _btnFinances.begin(); iter != _btnFinances.end(); ++iter)
		{
			(*iter)->setVisible(true);
		}
		_txtTitle->setBig();
		_txtTitle->setText(tr("STR_FINANCE"));

	}

	/**
	* Handles a click on a region button.
	* @param action Pointer to an action.
	*/
	void GraphsState::btnRegionListClick(Action * action)
	{
		size_t number = 0;
		ToggleTextButton *button = dynamic_cast<ToggleTextButton*>(action->getSender());

		if (button == _btnRegionTotal)
		{
			number = _btnRegions.size();
		}
		else
		{
			for (size_t i = 0; i < _btnRegions.size(); ++i)
			{
				if (button == _btnRegions[i])
				{
					number = i;
					break;
				}
			}
		}

		_regionToggles.at(number + _butRegionsOffset)->_pushed = button->getPressed();

		drawLines();
	}

	/**
	* Handles a click on a country button.
	* @param action Pointer to an action.
	*/
	void GraphsState::btnCountryListClick(Action * action)
	{
		size_t number = 0;
		ToggleTextButton *button = dynamic_cast<ToggleTextButton*>(action->getSender());

		if (button == _btnCountryTotal)
		{
			number = _btnCountries.size();
		}
		else
		{
			for (size_t i = 0; i < _btnCountries.size(); ++i)
			{
				if (button == _btnCountries[i])
				{
					number = i;
					break;
				}
			}
		}

		_countryToggles.at(number + _butCountriesOffset)->_pushed = button->getPressed();

		drawLines();
	}

	/**
	* handles a click on a finances button.
	* @param action Pointer to an action.
	*/
	void GraphsState::btnFinanceListClick(Action *action)
	{
		size_t number = 0;
		ToggleTextButton *button = dynamic_cast<ToggleTextButton*>(action->getSender());

		for (size_t i = 0; i < _btnFinances.size(); ++i)
		{
			if (button == _btnFinances[i])
			{
				number = i;
				break;
			}
		}

		_financeLines.at(number)->setVisible(!_financeToggles.at(number));
		_financeToggles.at(number) = button->getPressed();

		drawLines();
	}

	/**
	* remove all elements from view
	*/
	void GraphsState::resetScreen()
	{
		for (std::vector<Surface *>::iterator iter = _alienRegionLines.begin(); iter != _alienRegionLines.end(); ++iter)
		{
			(*iter)->setVisible(false);
		}
		for (std::vector<Surface *>::iterator iter = _alienCountryLines.begin(); iter != _alienCountryLines.end(); ++iter)
		{
			(*iter)->setVisible(false);
		}
		for (std::vector<Surface *>::iterator iter = _xcomRegionLines.begin(); iter != _xcomRegionLines.end(); ++iter)
		{
			(*iter)->setVisible(false);
		}
		for (std::vector<Surface *>::iterator iter = _xcomCountryLines.begin(); iter != _xcomCountryLines.end(); ++iter)
		{
			(*iter)->setVisible(false);
		}
		for (std::vector<Surface *>::iterator iter = _incomeLines.begin(); iter != _incomeLines.end(); ++iter)
		{
			(*iter)->setVisible(false);
		}
		for (std::vector<Surface *>::iterator iter = _financeLines.begin(); iter != _financeLines.end(); ++iter)
		{
			(*iter)->setVisible(false);
		}


		for (std::vector<ToggleTextButton *>::iterator iter = _btnRegions.begin(); iter != _btnRegions.end(); ++iter)
		{
			(*iter)->setVisible(false);
		}
		for (std::vector<ToggleTextButton *>::iterator iter = _btnCountries.begin(); iter != _btnCountries.end(); ++iter)
		{
			(*iter)->setVisible(false);
		}
		for (std::vector<ToggleTextButton *>::iterator iter = _btnFinances.begin(); iter != _btnFinances.end(); ++iter)
		{
			(*iter)->setVisible(false);
		}

		_btnRegionTotal->setVisible(false);
		_btnCountryTotal->setVisible(false);
		_txtFactor->setVisible(false);
	}

	/**
	* updates the text on the vertical scale
	* @param lowerLimit minimum value
	* @param upperLimit maximum value
	*/
	void GraphsState::updateScale(double lowerLimit, double upperLimit)
	{
		double increment = ((upperLimit - lowerLimit) / 9);
		if (increment < 10)
		{
			increment = 10;
		}
		double text = lowerLimit;
		for (int i = 0; i < 10; ++i)
		{
			_txtScale.at(i)->setText(Text::formatNumber(static_cast<int>(text)));
			text += increment;
		}
	}

	/**
	* instead of having all our line drawing in one giant ridiculous routine, just use the one we need.
	*/
	void GraphsState::drawLines()
	{
		if (!_country && !_finance)
		{
			drawRegionLines();
		}
		else if (!_finance)
		{
			drawCountryLines();
		}
		else
		{
			drawFinanceLines();
		}
	}

	/**
	* Sets up the screens and draws the lines for country buttons
	* to toggle on and off
	*/
	void GraphsState::drawCountryLines()
	{
		//calculate the totals, and set up our upward maximum
		int upperLimit = 0;
		int lowerLimit = 0;
		int totals[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		for (size_t entry = 0; entry != _game->getSavedGame()->getFundsList().size(); ++entry)
		{
			int total = 0;
			if (_alien)
			{
				for (size_t iter = 0; iter != _game->getSavedGame()->getCountries()->size(); ++iter)
				{
					total += _game->getSavedGame()->getCountries()->at(iter)->getActivityAlien().at(entry);
					if (_game->getSavedGame()->getCountries()->at(iter)->getActivityAlien().at(entry) > upperLimit && _countryToggles.at(iter)->_pushed)
					{
						upperLimit = _game->getSavedGame()->getCountries()->at(iter)->getActivityAlien().at(entry);
					}
				}
			}
			else if (_income)
			{
				for (size_t iter = 0; iter != _game->getSavedGame()->getCountries()->size(); ++iter)
				{
					total += _game->getSavedGame()->getCountries()->at(iter)->getFunding().at(entry) / 1000;
					if (_game->getSavedGame()->getCountries()->at(iter)->getFunding().at(entry) / 1000 > upperLimit && _countryToggles.at(iter)->_pushed)
					{
						upperLimit = _game->getSavedGame()->getCountries()->at(iter)->getFunding().at(entry) / 1000;
					}
				}
			}
			else
			{
				for (size_t iter = 0; iter != _game->getSavedGame()->getCountries()->size(); ++iter)
				{
					total += _game->getSavedGame()->getCountries()->at(iter)->getActivityXcom().at(entry);
					if (_game->getSavedGame()->getCountries()->at(iter)->getActivityXcom().at(entry) > upperLimit && _countryToggles.at(iter)->_pushed)
					{
						upperLimit = _game->getSavedGame()->getCountries()->at(iter)->getActivityXcom().at(entry);
					}
					if (_game->getSavedGame()->getCountries()->at(iter)->getActivityXcom().at(entry) < lowerLimit && _countryToggles.at(iter)->_pushed)
					{
						lowerLimit = _game->getSavedGame()->getCountries()->at(iter)->getActivityXcom().at(entry);
					}

				}
			}
			if (_countryToggles.back()->_pushed && total > upperLimit)
				upperLimit = total;
		}

		//adjust the scale to fit the upward maximum
		double range = upperLimit - lowerLimit;
		double low = lowerLimit;
		int grids = 9; // cells in grid
		if (low<0) grids--;
		int check = _income ? 50 : 10;
		while (range > check * grids)
		{
			check *= 2;
		}

		lowerLimit = 0;
		upperLimit = check * grids;

		if (low < 0)
		{
			while (low < lowerLimit)
			{
				lowerLimit -= check;
				upperLimit -= check;
			}
		}

		range = upperLimit - lowerLimit;
		double units = range / 126;

		// draw country lines *** HERE IS WHERE THE SHIT HITS THE FAN ***
		for (size_t entry = 0; entry != _game->getSavedGame()->getCountries()->size(); ++entry)
		{
			Country *country = _game->getSavedGame()->getCountries()->at(entry);
			_alienCountryLines.at(entry)->clear();
			_xcomCountryLines.at(entry)->clear();
			_incomeLines.at(entry)->clear();
			std::vector<Sint16> newLineVector;
			int reduction = 0;
			for (size_t iter = 0; iter != 12; ++iter)
			{
				int x = 560 - (iter * 35); // TEST 312 -> 560; iter*17 is where our nodes are so need to make it 35
				int y = 275 - (-lowerLimit / units); //TEST 175 -> 275
				if (_alien)
				{
					if (iter < country->getActivityAlien().size())
					{
						reduction = country->getActivityAlien().at(country->getActivityAlien().size() - (1 + iter)) / units;
						y -= reduction;
						totals[iter] += country->getActivityAlien().at(country->getActivityAlien().size() - (1 + iter));
					}
				}
				else if (_income)
				{
					if (iter < country->getFunding().size())
					{
						reduction = (country->getFunding().at(country->getFunding().size() - (1 + iter)) / 1000) / units;
						y -= reduction;
						totals[iter] += country->getFunding().at(country->getFunding().size() - (1 + iter)) / 1000;
					}
				}
				else
				{
					if (iter < country->getActivityXcom().size())
					{
						reduction = country->getActivityXcom().at(country->getActivityXcom().size() - (1 + iter)) / units;
						y -= reduction;
						totals[iter] += country->getActivityXcom().at(country->getActivityXcom().size() - (1 + iter));
					}
				}
				if (y >= 175)
					y = 175;
				newLineVector.push_back(y);
				int offset = 0;
				if (entry % 2)
					offset = 8;
				if (newLineVector.size() > 1 && _alien)
					_alienCountryLines.at(entry)->drawLine(x, y, x + 35, newLineVector.at(newLineVector.size() - 2), Palette::blockOffset((entry / 2) + 1) + offset); //changing all 17 -> 35
				else if (newLineVector.size() > 1 && _income)
					_incomeLines.at(entry)->drawLine(x, y, x + 35, newLineVector.at(newLineVector.size() - 2), Palette::blockOffset((entry / 2) + 1) + offset); //changing all 17 -> 35
				else if (newLineVector.size() > 1)
					_xcomCountryLines.at(entry)->drawLine(x, y, x + 35, newLineVector.at(newLineVector.size() - 2), Palette::blockOffset((entry / 2) + 1) + offset); //changing all 17 -> 35
			}
			if (_alien)
				_alienCountryLines.at(entry)->setVisible(_countryToggles.at(entry)->_pushed);
			else if (_income)
				_incomeLines.at(entry)->setVisible(_countryToggles.at(entry)->_pushed);
			else
				_xcomCountryLines.at(entry)->setVisible(_countryToggles.at(entry)->_pushed);
		}
		if (_alien)
			_alienCountryLines.back()->clear();
		else if (_income)
			_incomeLines.back()->clear();
		else
			_xcomCountryLines.back()->clear();

		// set up the "total" line
		std::vector<Sint16> newLineVector;
		Uint8 color = _game->getMod()->getInterface("graphs")->getElement("countryTotal")->color2;
		for (int iter = 0; iter != 12; ++iter)
		{
			int x = 312 - (iter * 35);
			int y = 175 - (-lowerLimit / units);
			if (totals[iter] > 0)
			{
				int reduction = totals[iter] / units;
				y -= reduction;
			}
			newLineVector.push_back(y);
			if (newLineVector.size() > 1)
			{
				if (_alien)
					_alienCountryLines.back()->drawLine(x, y, x + 35, newLineVector.at(newLineVector.size() - 2), color); //changing all 17 -> 35
				else if (_income)
					_incomeLines.back()->drawLine(x, y, x + 35, newLineVector.at(newLineVector.size() - 2), color); //changing all 17 -> 35
				else
					_xcomCountryLines.back()->drawLine(x, y, x + 35, newLineVector.at(newLineVector.size() - 2), color); //changing all 17 -> 35
			}
		}
		if (_alien)
			_alienCountryLines.back()->setVisible(_countryToggles.back()->_pushed);
		else if (_income)
			_incomeLines.back()->setVisible(_countryToggles.back()->_pushed);
		else
			_xcomCountryLines.back()->setVisible(_countryToggles.back()->_pushed);
		updateScale(lowerLimit, upperLimit);
		_txtFactor->setVisible(_income);
	}

	/**
	* Sets up the screens and draws the lines for region buttons
	* to toggle on and off
	*/
	void GraphsState::drawRegionLines()
	{
		//calculate the totals, and set up our upward maximum
		int upperLimit = 0;
		int lowerLimit = 0;
		int totals[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		for (size_t entry = 0; entry != _game->getSavedGame()->getFundsList().size(); ++entry)
		{
			int total = 0;
			if (_alien)
			{
				for (size_t iter = 0; iter != _game->getSavedGame()->getRegions()->size(); ++iter)
				{
					total += _game->getSavedGame()->getRegions()->at(iter)->getActivityAlien().at(entry);
					if (_game->getSavedGame()->getRegions()->at(iter)->getActivityAlien().at(entry) > upperLimit && _regionToggles.at(iter)->_pushed)
					{
						upperLimit = _game->getSavedGame()->getRegions()->at(iter)->getActivityAlien().at(entry);
					}
					if (_game->getSavedGame()->getRegions()->at(iter)->getActivityAlien().at(entry) < lowerLimit && _regionToggles.at(iter)->_pushed)
					{
						lowerLimit = _game->getSavedGame()->getRegions()->at(iter)->getActivityAlien().at(entry);
					}
				}
			}
			else
			{
				for (size_t iter = 0; iter != _game->getSavedGame()->getRegions()->size(); ++iter)
				{
					total += _game->getSavedGame()->getRegions()->at(iter)->getActivityXcom().at(entry);
					if (_game->getSavedGame()->getRegions()->at(iter)->getActivityXcom().at(entry) > upperLimit && _regionToggles.at(iter)->_pushed)
					{
						upperLimit = _game->getSavedGame()->getRegions()->at(iter)->getActivityXcom().at(entry);
					}
					if (_game->getSavedGame()->getRegions()->at(iter)->getActivityXcom().at(entry) < lowerLimit && _regionToggles.at(iter)->_pushed)
					{
						lowerLimit = _game->getSavedGame()->getRegions()->at(iter)->getActivityXcom().at(entry);
					}
				}
			}
			if (_regionToggles.back()->_pushed && total > upperLimit)
				upperLimit = total;
		}

		//adjust the scale to fit the upward maximum
		double range = upperLimit - lowerLimit;
		double low = lowerLimit;
		int check = 10;
		int grids = 9; // cells in grid
		if (low<0) grids--;
		while (range > check * grids)
		{
			check *= 2;
		}

		lowerLimit = 0;
		upperLimit = check * 9;

		if (low < 0)
		{
			while (low < lowerLimit)
			{
				lowerLimit -= check;
				upperLimit -= check;
			}
		}
		range = upperLimit - lowerLimit;
		double units = range / 126;
		// draw region lines
		for (size_t entry = 0; entry != _game->getSavedGame()->getRegions()->size(); ++entry)
		{
			Region *region = _game->getSavedGame()->getRegions()->at(entry);
			_alienRegionLines.at(entry)->clear();
			_xcomRegionLines.at(entry)->clear();
			std::vector<Sint16> newLineVector;
			int reduction = 0;
			for (size_t iter = 0; iter != 12; ++iter)
			{
				int x = 312 - (iter * 35);
				int y = 175 - (-lowerLimit / units);
				if (_alien)
				{
					if (iter < region->getActivityAlien().size())
					{
						reduction = region->getActivityAlien().at(region->getActivityAlien().size() - (1 + iter)) / units;
						y -= reduction;
						totals[iter] += region->getActivityAlien().at(region->getActivityAlien().size() - (1 + iter));
					}
				}
				else
				{
					if (iter < region->getActivityXcom().size())
					{
						reduction = region->getActivityXcom().at(region->getActivityXcom().size() - (1 + iter)) / units;
						y -= reduction;
						totals[iter] += region->getActivityXcom().at(region->getActivityXcom().size() - (1 + iter));
					}
				}
				if (y >= 175)
					y = 175;
				newLineVector.push_back(y);
				int offset = 0;
				if (entry % 2)
					offset = 8;
				if (newLineVector.size() > 1 && _alien)
					_alienRegionLines.at(entry)->drawLine(x, y, x + 35, newLineVector.at(newLineVector.size() - 2), Palette::blockOffset((entry / 2) + 1) + offset);
				else if (newLineVector.size() > 1)
					_xcomRegionLines.at(entry)->drawLine(x, y, x + 35, newLineVector.at(newLineVector.size() - 2), Palette::blockOffset((entry / 2) + 1) + offset);
			}

			if (_alien)
				_alienRegionLines.at(entry)->setVisible(_regionToggles.at(entry)->_pushed);
			else
				_xcomRegionLines.at(entry)->setVisible(_regionToggles.at(entry)->_pushed);
		}

		// set up the "total" line
		if (_alien)
			_alienRegionLines.back()->clear();
		else
			_xcomRegionLines.back()->clear();

		Uint8 color = _game->getMod()->getInterface("graphs")->getElement("regionTotal")->color2;
		std::vector<Sint16> newLineVector;
		for (int iter = 0; iter != 12; ++iter)
		{
			int x = 312 - (iter * 35);
			int y = 175;
			if (totals[iter] > 0)
			{
				int reduction = totals[iter] / units;
				y -= reduction;
			}
			newLineVector.push_back(y);
			if (newLineVector.size() > 1)
			{
				if (_alien)
					_alienRegionLines.back()->drawLine(x, y, x + 35, newLineVector.at(newLineVector.size() - 2), color); //changing all 17 -> 35
				else
					_xcomRegionLines.back()->drawLine(x, y, x + 35, newLineVector.at(newLineVector.size() - 2), color); //changing all 17 -> 35
			}
		}
		if (_alien)
			_alienRegionLines.back()->setVisible(_regionToggles.back()->_pushed);
		else
			_xcomRegionLines.back()->setVisible(_regionToggles.back()->_pushed);
		updateScale(lowerLimit, upperLimit);
		_txtFactor->setVisible(false);
	}

	/**
	* Sets up the screens and draws the lines for the finance buttons
	* to toggle on and off
	*/
	void GraphsState::drawFinanceLines()
	{
		//set up arrays
		int upperLimit = 0;
		int lowerLimit = 0;
		int incomeTotals[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		int balanceTotals[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		int expendTotals[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		int maintTotals[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		int scoreTotals[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		maintTotals[0] = _game->getSavedGame()->getBaseMaintenance() / 1000;

		// start filling those arrays with score values
		// determine which is the highest one being displayed, so we can adjust the scale
		for (size_t entry = 0; entry != _game->getSavedGame()->getFundsList().size(); ++entry)
		{
			size_t invertedEntry = _game->getSavedGame()->getFundsList().size() - (1 + entry);
			maintTotals[entry] += _game->getSavedGame()->getMaintenances().at(invertedEntry) / 1000;
			balanceTotals[entry] = _game->getSavedGame()->getFundsList().at(invertedEntry) / 1000;
			scoreTotals[entry] = _game->getSavedGame()->getResearchScores().at(invertedEntry);

			for (std::vector<Region*>::iterator iter = _game->getSavedGame()->getRegions()->begin(); iter != _game->getSavedGame()->getRegions()->end(); ++iter)
			{
				scoreTotals[entry] += (*iter)->getActivityXcom().at(invertedEntry) - (*iter)->getActivityAlien().at(invertedEntry);
			}

			if (_financeToggles.at(2))
			{
				if (maintTotals[entry] > upperLimit)
				{
					upperLimit = maintTotals[entry];
				}
				if (maintTotals[entry] < lowerLimit)
				{
					lowerLimit = maintTotals[entry];
				}
			}
			if (_financeToggles.at(3))
			{
				if (balanceTotals[entry] > upperLimit)
				{
					upperLimit = balanceTotals[entry];
				}
				if (balanceTotals[entry] < lowerLimit)
				{
					lowerLimit = balanceTotals[entry];
				}
			}
			if (_financeToggles.at(4))
			{
				if (scoreTotals[entry] > upperLimit)
				{
					upperLimit = scoreTotals[entry];
				}
				if (scoreTotals[entry] < lowerLimit)
				{
					lowerLimit = scoreTotals[entry];
				}
			}
		}

		for (size_t entry = 0; entry != _game->getSavedGame()->getExpenditures().size(); ++entry)
		{
			expendTotals[entry] = _game->getSavedGame()->getExpenditures().at(_game->getSavedGame()->getExpenditures().size() - (entry + 1)) / 1000;
			incomeTotals[entry] = _game->getSavedGame()->getIncomes().at(_game->getSavedGame()->getIncomes().size() - (entry + 1)) / 1000;

			if (_financeToggles.at(0) && incomeTotals[entry] > upperLimit)
			{
				upperLimit = incomeTotals[entry];
			}
			if (_financeToggles.at(1) && expendTotals[entry] > upperLimit)
			{
				upperLimit = expendTotals[entry];
			}
		}

		double range = upperLimit - lowerLimit;
		double low = lowerLimit;
		int check = 250;
		int grids = 9; // cells in grid
		if (low<0) grids--;
		while (range > check * grids)
		{
			check *= 2;
		}

		lowerLimit = 0;
		upperLimit = check * 9;

		if (low < 0)
		{
			while (low < lowerLimit)
			{
				lowerLimit -= check;
				upperLimit -= check;
			}
		}
		//toggle screens
		for (int button = 0; button != 5; ++button)
		{
			_financeLines.at(button)->setVisible(_financeToggles.at(button));
			_financeLines.at(button)->clear();
		}
		range = upperLimit - lowerLimit;
		//figure out how many units to the pixel, then plot the points for the graph and connect the dots.
		double units = range / 126;
		for (int button = 0; button != 5; ++button)
		{
			std::vector<Sint16> newLineVector;
			for (int iter = 0; iter != 12; ++iter)
			{
				int x = 312 - (iter * 17);
				int y = 175 - (-lowerLimit / units);
				int reduction = 0;
				switch (button)
				{
				case 0:
					reduction = incomeTotals[iter] / units;
					break;
				case 1:
					reduction = expendTotals[iter] / units;
					break;
				case 2:
					reduction = maintTotals[iter] / units;
					break;
				case 3:
					reduction = balanceTotals[iter] / units;
					break;
				case 4:
					reduction = scoreTotals[iter] / units;
					break;
				}
				y -= reduction;
				newLineVector.push_back(y);
				int offset = button % 2 ? 8 : 0;
				if (newLineVector.size() > 1)
					_financeLines.at(button)->drawLine(x, y, x + 17, newLineVector.at(newLineVector.size() - 2), Palette::blockOffset((button / 2) + 1) + offset);
			}
		}
		updateScale(lowerLimit, upperLimit);
		_txtFactor->setVisible(true);
	}

	/**
	* 'Shift' the buttons to display only GRAPH_MAX_BUTTONS - reset their state from toggles
	*/
	void GraphsState::shiftButtons(Action *action)
	{
		// only if active 'screen' is other than finance
		if (_finance)
			return;
		// select the data's we'll processing - regions or countries
		if (_country)
		{
			// too few countries? - return
			if (_countryToggles.size() <= GRAPH_MAX_BUTTONS)
				return;
			else if (action->getDetails()->button.button == SDL_BUTTON_WHEELUP)
				scrollButtons(_countryToggles, _btnCountries, _butCountriesOffset, -1);
			else if (action->getDetails()->button.button == SDL_BUTTON_WHEELDOWN)
				scrollButtons(_countryToggles, _btnCountries, _butCountriesOffset, 1);
		}
		else
		{
			// too few regions? - return
			if (_regionToggles.size() <= GRAPH_MAX_BUTTONS)
				return;
			else if (action->getDetails()->button.button == SDL_BUTTON_WHEELUP)
				scrollButtons(_regionToggles, _btnRegions, _butRegionsOffset, -1);
			else if (action->getDetails()->button.button == SDL_BUTTON_WHEELDOWN)
				scrollButtons(_regionToggles, _btnRegions, _butRegionsOffset, 1);
		}
	}
	void GraphsState::scrollButtons(std::vector<GraphButInfo *> &toggles, std::vector<ToggleTextButton *> &buttons, size_t &offset, int step)
	{
		// minus one, 'cause we'll already added the TOTAL button to toggles
		if (int(step + (int)offset) < 0 || offset + step + GRAPH_MAX_BUTTONS >= toggles.size() - 1)
			return;
		// set the next offset - cheaper to do it from starters
		offset += step;
		size_t i = 0;
		std::vector<ToggleTextButton *>::iterator iterb = buttons.begin();
		for (std::vector<GraphButInfo *>::iterator itert = toggles.begin(); itert != toggles.end(); ++itert, ++i)
			if (i < offset)
				continue;
			else if (i < offset + GRAPH_MAX_BUTTONS)
				updateButton(*itert, *iterb++);
			else
				return;
	}
	void GraphsState::updateButton(GraphButInfo *from, ToggleTextButton *to)
	{
		to->setText(from->_name);
		to->setInvertColor(from->_color);
		to->setPressed(from->_pushed);
	}
}

#pragma once
#include "PopUpWindow.h"

class WindowBuild : public PopUpWindow
{
public:
	WindowBuild();
	~WindowBuild() override;

	bool StartedBuild() const;
	void Reset();

protected:
	void DrawWindowContents() override;

private:
	void DrawSceneComboBox();
	void DrawBuildTypeComboBox();
	void DrawGenerateZipCheckbox();
	void DrawBuildButton();

	std::string selectedScene;
	std::string selectedBuild;
	bool generateZip;

	bool startedBuild;
};

inline bool WindowBuild::StartedBuild() const
{
	return startedBuild;
}

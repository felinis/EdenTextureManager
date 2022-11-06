#include "imgui/imgui.h"
#include "levelResources.h"
#include "renderer.h"

void DrawUI()
{
#ifdef IMGUI_HAS_VIEWPORT
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->GetWorkPos());
	ImGui::SetNextWindowSize(viewport->GetWorkSize());
	ImGui::SetNextWindowViewport(viewport->ID);
#else 
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif

	if (!ImGui::Begin("Texture Preview", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize))
		return;
	ImGui::Text("Project Eden Texture Manager v1.1 - written by fenik0 - built on 24.07.2022");
	ImGui::Separator();
	ImGui::Text("This level file contains %u textures.", LevelResources::GetNumTextures());
	ImGui::Separator();
	ImGui::Text("Click on a texture to bring up a context menu.");
	ImGui::Text("You can scroll through the textures using your mouse wheel or the scroll bar on the right.");
	ImGui::Separator();
	static float currTextureSize = 128.0f;
	const float minTextureSize = 128.0f;
	const float maxTextureSize = 1024.0f;
	ImGui::SliderFloat("Preview size (in pixels)", &currTextureSize, minTextureSize, maxTextureSize,
		"%.1f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoRoundToFormat);

	ImVec2 size(currTextureSize, currTextureSize);
	u32 numCols = (u32)maxTextureSize / (u32)currTextureSize;
	ImGui::BeginTable("textureTable", numCols, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY);
#if 0
	u32 currTextureSlot = 0;
	const u32 numTextureSlots = LevelResources::GetTextureCount();
	u32 numRows = numTextureSlots / numCols + (numTextureSlots % numCols);
	for (u32 y = 0; y < numRows; y++)
	{
		ImGui::TableNextRow();
		for (u32 x = 0; x < numCols; x++)
		{
			ImGui::TableNextColumn();
			ImGui::BeginGroup();
			Texture *t = LevelResources::GetTexture(currTextureSlot++);
			void *imguiTex = Renderer::GetImGuiTexture(t->GetResourceID());
			ImGui::ImageButton(imguiTex, size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f)); /* tesktury Edena s¹ odwrócone */
			ImGui::Text("%s %ux%u", t->GetFormatString(), t->GetWidth(), t->GetHeight()); //ImGui::GetContentRegionAvail()
			ImGui::EndGroup();

//			if (x < 5)
//				ImGui::SameLine();
		}

	}
#elif 0
	u32 currTextureSlot = 0;
	static u32 selectedTextureSlot = 0;
	u32 numItemsInRow = (u32)maxTextureSize / (u32)currTextureSize;
	for (u32 i = 0; i < LevelResources::GetNumTextureSlots(); i++)
	{
		Texture& t = LevelResources::GetTexture(i);
		if (!t.IsValid())
			continue;

		u32 rowID = currTextureSlot % numItemsInRow;
//		u32 colID = currTextureSlot / numItemsInRow;

		if (rowID == 0)
			ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::BeginGroup();
		if (ImGui::ImageButton(0, size,
			ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f))) /* tesktury Edena s¹ odwrócone */
		{
			/* poka¿ menu kontekstowe */
			ImGui::OpenPopup("TPU");
			selectedTextureSlot = i;
		}
		
//		ImGui::Text("%u: %s %ux%u", i, t.GetFormatString(), t.GetWidth(), t.GetHeight());
		ImGui::EndGroup();

		currTextureSlot++;
	}

	if (ImGui::BeginPopup("TPU"))
	{
		/* show every mip */
		Texture& t = LevelResources::GetTexture(selectedTextureSlot);
		for (u32 i = 0; i < t.GetNumMips(); i++)
		{
			ImGui::Text("Child %u", i);
			TextureMip& mip = t.GetMip(i);
			ImGui::Image(Renderer::GetImGuiTexture(mip.GetResourceID()), size,
				ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}

		/*
		
		ImGui::ImageButton(Renderer::GetImGuiTexture(t.GetResourceID()), size,
//			ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f))
		
		*/

//		if (ImGui::MenuItem("Export"))
//			LevelResources::ExportTexture(selectedTextureSlot);

//		if (ImGui::MenuItem("Replace"))
//			LevelResources::ReplaceTexture(selectedTextureSlot);

		ImGui::EndPopup();
	}
#else
	u32 currTextureSlot = 0;
	u32 numItemsInRow = (u32)maxTextureSize / (u32)currTextureSize;

	static u32 selectedTextureSlot = 0;
	static u32 selectedFrameSlot = 0;

	/* for every texture */
	for (u32 textureID = 0; textureID < LevelResources::GetNumTextureSlots(); textureID++)
	{
		Texture& t = LevelResources::GetTexture(textureID);
		if (!t.IsValid())
			continue;

		/* draw every frame of animation */
		for (u32 frameID = 0; frameID < t.GetNumFrames(); frameID++)
		{
			TextureFrame& frame = t.GetFrame(frameID);

			u32 rowID = currTextureSlot % numItemsInRow;
			if (rowID == 0)
				ImGui::TableNextRow();

			ImGui::TableNextColumn();

			ImGui::BeginGroup();
			if (ImGui::ImageButton(Renderer::GetImGuiTexture(frame.GetResourceID()), size,
				ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f))) /* tesktury Edena s¹ odwrócone */
			{
				/* poka¿ menu kontekstowe */
				ImGui::OpenPopup("TPU");
				selectedTextureSlot = textureID;
				selectedFrameSlot = frameID;
			}
			ImGui::Text("S%uF%u: %s %ux%u", textureID, frameID, frame.GetFormatString(),
				frame.GetWidth(), frame.GetHeight());
			ImGui::EndGroup();

			currTextureSlot++;
		}
	}

	if (ImGui::BeginPopup("TPU"))
	{
		if (ImGui::MenuItem("Export"))
			LevelResources::ExportTexture(selectedTextureSlot, selectedFrameSlot);

		if (ImGui::MenuItem("Replace"))
			LevelResources::ReplaceTexture(selectedTextureSlot, selectedFrameSlot);

		ImGui::EndPopup();
	}
#endif

	ImGui::EndTable();

	ImGui::End();
}

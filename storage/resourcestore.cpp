#include "pch.h"
#include "resourcestore.h"
#include <extensions/Paths.h>
#include <CTxdStore.h>
#include "trainer.h"

ResourceStore::ResourceStore(const char* text, eResourceType type, ImVec2 imageSize)
    : m_ImageSize(imageSize), m_Type(type), m_FileName(text) {
    if (m_Type != eResourceType::TYPE_IMAGE) {
        std::string path{ PLUGIN_PATH((char*)(FILE_NAME"/data/" + std::string(text) + ".toml").c_str()) };
        m_pData = std::make_unique<ConfigStorage>(path);

        if (m_Type != eResourceType::TYPE_IMAGE_TEXT) {
            // Generate categories
            for (auto [k, v] : m_pData->Items()) {
                m_Categories.push_back(std::string(k.str()));
            }
            UpdateSearchList();
        }
    }

    if (m_Type != eResourceType::TYPE_TEXT) {
        /*
            Textures need to be loaded from main thread
            Loading it directly here doesn't work
            TODO:
                Maybe enabling a dx9 flag fixes this?
                Switch to initScriptsEvent
        */
        Events::drawingEvent += [text, this]() {
            if (!m_bTexturesLoaded) {
                if (gD3dDevice != nullptr) { // Delay loading until d3d is initialized
                    LoadTextureResource(text);
                    for (auto& e : m_nCustomList) {
                        if (e.second.isLoaded) {
                            std::string key = std::format("Custom.{}##Added", e.second.name);
                            m_pData->Set(key.c_str(), std::to_string(e.first));
                            e.second.isLoaded = false;
                        }
                    }
                    m_bTexturesLoaded = true;
                }
            }
            };
    }
}

// Get dx9 texture object from RwTexture*
void* GetTextureFromRaster(RwTexture* pTexture) {
    if (gRenderer == eRenderer::Dx9) {
        IDirect3DTexture9* d3dTex = nullptr;
        if (pTexture == nullptr) {
            return nullptr;
        }

        int width = pTexture->raster->width;
        int height = pTexture->raster->height;

        if (!gD3dDevice) {
            gD3dDevice = GetD3DDevice();
        }

        HRESULT hr = reinterpret_cast<IDirect3DDevice9*>(gD3dDevice)->CreateTexture(width, height, 1, 0, D3DFMT_DXT5, D3DPOOL_MANAGED, &d3dTex, nullptr);
        if (FAILED(hr)) {
            return nullptr;
        }

        D3DLOCKED_RECT lockedRect;
        hr = d3dTex->LockRect(0, &lockedRect, nullptr, 0);
        if (FAILED(hr)) {
            d3dTex->Release();
            return nullptr;
        }

        unsigned char* srcData = RwRasterLock(pTexture->raster, 0, pTexture->raster->cFormat);
        unsigned char* destData = static_cast<unsigned char*>(lockedRect.pBits);
        D3DSURFACE_DESC desc;
        d3dTex->GetLevelDesc(0, &desc);
        memcpy(destData, srcData, desc.Width * desc.Height);
        RwRasterUnlock(pTexture->raster);
        d3dTex->UnlockRect(0);
        return d3dTex;
    }
    else if (gRenderer == eRenderer::Dx11) {
     // D3D11_TEXTURE2D_DESC desc = {};
     // desc.Width = pTexture->raster->width;
     // desc.Height = pTexture->raster->height;
     // desc.MipLevels = 1;
     // desc.ArraySize = 1;
     // desc.Format = DXGI_FORMAT_BC3_UNORM; // BC3 format (DXT5)
     // desc.SampleDesc.Count = 1;
     // desc.Usage = D3D11_USAGE_DEFAULT;
     // desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
     // desc.CPUAccessFlags = 0;

     // // Copy data to texture
     // D3D11_SUBRESOURCE_DATA initData = {};
     // initData.pSysMem = RwRasterLock(pTexture->raster, 0, pTexture->raster->cFormat);
     // initData.SysMemPitch = ((pTexture->raster->width + 3) / 4) * 16;
     // initData.SysMemSlicePitch = 0;

     // ID3D11Texture2D* d3dTex = nullptr;
     // HRESULT hr = reinterpret_cast<ID3D11Device*>(gD3dDevice)->CreateTexture2D(&desc, &initData, &d3dTex);
     // RwRasterUnlock(pTexture->raster);

     // if (FAILED(hr)) {
     //     return nullptr;
     // }

     // // Create shader resource view
     // D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
     // srvDesc.Format = desc.Format;
     // srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
     // srvDesc.Texture2D.MostDetailedMip = 0;
     // srvDesc.Texture2D.MipLevels = desc.MipLevels;

     // ID3D11ShaderResourceView* texture_srv = nullptr;
     // hr = reinterpret_cast<ID3D11Device*>(gD3dDevice)->CreateShaderResourceView(d3dTex, &srvDesc, &texture_srv);
     // if (FAILED(hr)) {
     //     d3dTex->Release();
     //     return nullptr;
     // }
     // d3dTex->Release();

     // return texture_srv;
    }

    return nullptr;
}

RwTexture* ResourceStore::FindRwTextureByName(const std::string& name) {
    for (auto& item : m_ImagesList) {
        if (item->m_FileName == name) {
            return item->m_pRwTexture;
        }
    }
    return nullptr;
}

IDirect3DTexture9** ResourceStore::FindTextureByName(const std::string& name) {
    for (auto& item : m_ImagesList) {
        if (item->m_FileName == name) {
            return static_cast<IDirect3DTexture9**>(item->m_pTexture);
        }
    }
    return nullptr;
}

RwTexDictionary* LoadTexDictionary(const char* path)
{
#ifdef GTASA
    return plugin::CallAndReturnDynGlobal<RwTexDictionary*, char const*>(0x5B3860, path);
#else
    RwTexDictionary* pTex = nullptr;

    RwStream* pStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    if (pStream) {
        if (RwStreamFindChunk(pStream, 22, 0, 0)) {
            pTex = CallAndReturn<RwTexDictionary*, BY_GAME(NULL, 0x61E710, 0x5924A0)>(pStream); // RwTexDictionaryGtaStreamRead(pStream);
        }

        RwStreamClose(pStream, 0);
    }

    return pTex ? pTex : RwTexDictionaryCreate();
#endif
}


void ResourceStore::LoadTextureResource(std::string&& name) {

    std::string fullPath = PLUGIN_PATH((char*)FILE_NAME "\\") + name + ".txd";

    if (!std::filesystem::exists(fullPath)) {
        gLogger->warn("Failed to load {}", fullPath);
        return;
    }

    RwTexDictionary* pRwTexDictionary = LoadTexDictionary(fullPath.c_str());
    if (pRwTexDictionary) {
        RwLinkList* objectList = &pRwTexDictionary->texturesInDict;
        if (!rwLinkListEmpty(objectList))
        {
            bool addCategories = (m_Categories.size() < 3); // "All", "Custom"
            RwTexture* texture;
            RwLLLink* current = rwLinkListGetFirstLLLink(objectList);
            RwLLLink* end = rwLinkListGetTerminator(objectList);

            current = rwLinkListGetFirstLLLink(objectList);
            while (current != end)
            {
                texture = rwLLLinkGetData(current, RwTexture, lInDictionary);

                m_ImagesList.push_back(std::make_unique<TextureResource>());
                m_ImagesList.back().get()->m_pRwTexture = texture;

                // Fetch IDirec9Texture9* from RwTexture*
                m_ImagesList.back().get()->m_pTexture = GetTextureFromRaster(texture);

                // Naming format in Txd `Category$TextureName`
                std::stringstream ss(texture->name);
                std::string str;

                getline(ss, str, '$');

                if (m_Type == TYPE_TEXT_IMAGE) {
                    // generate categories from text data
                    for (auto [k, v] : m_pData->Items()) {
                        std::string val = v.value_or<std::string>("Unknown");
                        if (val == str) {
                            m_ImagesList.back().get()->m_CategoryName = k.str();
                            break;
                        }
                    }
                }
                else {
                    m_ImagesList.back().get()->m_CategoryName = str;
                }

                if (name == "clothes") {
                    // pass full name
                    m_ImagesList.back().get()->m_FileName = texture->name;
                }
                else {
                    getline(ss, str, '$');
                    m_ImagesList.back().get()->m_FileName = str;

                    // check if it's a number
                    if (!str.empty() && std::all_of(str.begin(), str.end(), ::isdigit)) {
                        m_nCustomList[std::stoi(str)].isLoaded = false;
                    }
                }

                // Genereate categories
                if (m_Type != TYPE_TEXT_IMAGE &&
                        !std::count(m_Categories.begin(), m_Categories.end(), m_ImagesList.back().get()->m_CategoryName)) {
                    m_Categories.push_back(m_ImagesList.back().get()->m_CategoryName);
                }

                current = rwLLLinkGetNext(current);
            }
        }
    }

    m_bSearchListUpdateRequired = true;
}

void ResourceStore::UpdateSearchList(bool favourites, fRtnArg1_t getNameFunc, fRtnBoolArg1_t verifyFunc) {
    m_nSearchList.clear();
    if (favourites) {
        if (m_Type == eResourceType::TYPE_TEXT) {
            for (auto [key, val] : *m_pData->GetTable("Favourites")) {
                ListLookup lookup;
                lookup.key = std::string(key.str());
                if (m_Filter.PassFilter(lookup.key.c_str())) {
                    lookup.cat = "Favourites";
                    lookup.val = val.value_or<std::string>("Unkonwn");
                    m_nSearchList.push_back(std::move(lookup));
                }
            }
        }
        else {
            for (auto [key, val] : *m_pData->GetTable("Favourites")) {
                bool isFound = false;
                std::string favName = std::string(key.str());
                std::string favModel = val.as_string()->value_or("0");

                for (uint i = 0; i < m_ImagesList.size(); ++i) {
                    ImageLookup lookup;
                    lookup.m_FileName = m_ImagesList[i]->m_FileName;
                    lookup.m_ModelName = getNameFunc == nullptr ? "" : getNameFunc(lookup.m_FileName);

                    if (lookup.m_ModelName == favName && m_Filter.PassFilter(lookup.m_ModelName.c_str())
                            && (verifyFunc == nullptr || verifyFunc(lookup.m_FileName))) {
                        lookup.m_bCustom = lookup.m_FileName.find("Added") != std::string::npos;
                        lookup.m_pTexture = m_ImagesList[i]->m_pTexture;
                        m_nSearchList.push_back(std::move(lookup));
                        isFound = true;
                        break;
                    }
                }

                if (isFound) {
                    continue;
                }

                for (auto [k, v] : *m_pData->GetTable("Custom")) {
                    std::string customName = std::string(k.str());
                    std::string customModel = v.as_string()->value_or("0");

                    if (favModel == customModel && m_Filter.PassFilter(customName.c_str())
                    && (m_Selected == "Custom" || m_Selected == "All")) {
                        ImageLookup lookup;
                        lookup.m_FileName = customModel;
                        lookup.m_ModelName = customName;
                        lookup.m_bCustom = true;
                        m_nSearchList.push_back(std::move(lookup));
                    }
                }
            }
        }
    }
    else {
        if (m_Type == eResourceType::TYPE_TEXT) {
            for (auto [cat, table] : m_pData->Items()) {
                // Don't show favourites in "All"
                if (m_Selected == "All" && cat == "Favourites") {
                    continue;
                }
                if (cat.str() == m_Selected || m_Selected == "All") {
                    if (!table.as_table()) {
                        return;
                    }
                    for (auto [key, val] : *table.as_table()->as_table()) {
                        ListLookup lookup;
                        lookup.key = std::string(key.str());
                        if (m_Filter.PassFilter(lookup.key.c_str())) {
                            lookup.cat = cat.str();
                            lookup.val = val.value_or<std::string>("Unkonwn");
                            m_nSearchList.push_back(std::move(lookup));
                        }
                    }
                }
            }
        }
        else {
            for (uint i = 0; i < m_ImagesList.size(); ++i) {
                ImageLookup lookup;
                lookup.m_FileName = m_ImagesList[i]->m_FileName;
                lookup.m_ModelName = getNameFunc == nullptr ? "" : getNameFunc(lookup.m_FileName);

                if (m_Filter.PassFilter(lookup.m_ModelName.c_str())
                        && (m_ImagesList[i]->m_CategoryName == m_Selected || m_Selected == "All")
                        && (verifyFunc == nullptr || verifyFunc(lookup.m_FileName))
                   ) {
                    lookup.m_bCustom = false;
                    lookup.m_pTexture = m_ImagesList[i]->m_pTexture;
                    m_nSearchList.push_back(std::move(lookup));
                }
            }
            for (auto [k, v] : *m_pData->GetTable("Custom")) {
                ImageLookup lookup;
                lookup.m_FileName = v.as_string()->value_or("0");
                lookup.m_ModelName = std::string(k.str());
                if (m_Filter.PassFilter(lookup.m_ModelName.c_str())
                        && (m_Selected == "Custom" || m_Selected == "All")) {
                    lookup.m_bCustom = true;
                    m_nSearchList.push_back(std::move(lookup));
                }
            }
        }
    }
    m_nSearchList.shrink_to_fit();
}
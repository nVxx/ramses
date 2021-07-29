//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTCACHE_H
#define RAMSES_TEXTCACHE_H

#include "ramses-text-api/TextLine.h"
#include "ramses-text-api/FontInstanceOffsets.h"
#include <string>

namespace ramses
{
    class Scene;
    class Effect;
    class IFontAccessor;
    class TextCacheImpl;

    /**
    * @brief Stores text data - texture atlas, meshes, glyph bitmap data. It is a cache because the
    * content can be re-generated when necessary, e.g. when cached glyphs take up too much memory.
    *
    * The TextCache class keeps hold of following data:
    * - The texture atlas pages (array of textures) which holds the glyph bitmaps for different glyphs
    * - The scene objects which represent a "text line" - an array of sized and positioned characters
    *
    * The TextCache uses IFontAccessor (provided in constructor) to obtain glyph data for various fonts
    * but has otherwise no dependencies to fonts - it treats glyphs as a simple bitmap image placed in
    * one of the texture atlas pages.
    *
    * The scene objects created for each texture atlas page with glyphs is:
    * - A 2D texture with a single channel (intensity of text pixel)
    * - A texture sampler with bilinear filtering (to blur out edges)
    *
    * The scene objects created for each text line are:
    * - a GeometryBinding with Vertex/Index arrays (holding a list of quads for each glyph in the text line)
    * - an Appearance (based off the effect provided in createTextLine()). The appearance will have it's semantic texture sampler
    *       pointing to the texture atlas page where the text line's glyphs are located
    * - a MeshNode which holds above objects together for rendering
    *
    * It's important to note that the Appearance of each TextLine has a reference to the texture page holding its
    * glyph data, and the GeometryBinding has links to the texture quad data generated by TextCache. We highly recommend not to
    * tamper with these objects, unless you have in-depth understanding of how text rendering works and know what you are
    * doing. However, setting custom uniforms in the Appearance is valid, as long as you are not touching the semantic texture
    * sampler which was provided when creating the text line.
    *
    * The TextCache has a limitation that all characters of a TextLine must fit in one (empty) texture atlas page.
    * This means that if a TextCache was created with size 16x16 and a TextLine with single glyph of size 32 is created,
    * the call to createTextLine() will fail because it can't fit a single glyph in a texture atlas page - it's too small.
    * Therefore, pay special attention to the size of TextCache and the size of text rendered. If you want to optimize memory
    * usage across different and wildly heterogeneous text sizes, it's suggested to use multiple TextCache instances with
    * different sizes if in order to avoid inefficient memory partitioning.
    *
    * To understand better how TextCache internally works, consider following case.
    * You create a TextCache with size 20x20. You use a latin font with uniform character size 10x10 for all letters.
    * You can thus only create TextLine's with a maximum of 4 letters. If you create two lines with size 4 which use the same
    * 4 characters, the TextCache will create a single texture page and use the glyphs for both lines. If you create two lines
    * with different, but partially overlapping characters, like this:
    * line1 = 'ABC'
    * line2 = 'AXY'
    *
    * the TextCache will not be able to create a page with all 5 characters (A, B, C, X and Y) and will need to create two pages.
    * The first page will put the letters 'ABC' in one page which will have one empty slot. The second line will not fit in the
    * first page, so the TextCache will create a second page and put the characters 'AXY' there. Note that the character 'A'
    * will be in in both pages, so that each line can be rendered with a single draw call using exactly one mesh and one texture.
    * One may argue that copying glyphs across different pages is bad, but trying to implement a TextCache which creates
    * multiple MeshNodes and partitions them in the worst case across all texture pages quickly leads to the awareness that it's
    * much better to just forbid it and sacriface a bit of memory in favor of much simpler implementation.
    *
    * The above limitation ensures that each TextLine receives exactly one MeshNode which makes the rendering setup much easier.
    * However, it requires that multi-language texts with large amount of glyphs may result in suboptimal texture atlas layout.
    * This memory overhead can be overcome by either using a texture atlas size large enough to hold any text line
    * or implementing a more sophisticated partitioning of TextLine's to ensure atlas pages are filled proportionally.
    *
    * Finally, the TextLine object holds pointers to the original vector of glyphs which were used to create it.
    * Be careful to not tamper with it, as it is used when destroying the text line to obtain information which
    * glyphs can be freed.
    */
    class RAMSES_API TextCache
    {
    public:
        /**
        * @brief Constructor for text cache.
        *
        * Choose carefully the size of the atlas textures. Too small will prevent creation of
        * larger strings, because not all of the glyphs will fit on a single page. Too large
        * pages take up more memory than actually needed.
        *
        * @param[in] scene Scene to use when creating meshes from string glyphs.
        * @param[in] fontAccessor Font accessor to be used for getting font instance objects
        * @param[in] atlasTextureWidth Width for the texture atlas that gets created to store glyphs
        * @param[in] atlasTextureHeight Height for the texture atlas that gets created to store glyphs
        */
        TextCache(Scene& scene, IFontAccessor& fontAccessor, uint32_t atlasTextureWidth, uint32_t atlasTextureHeight);

        /**
        * @brief Destructor for text cache that cleans up any objects created using the text cache.
        *        The scene object passed to the text cache constructor must be still valid at the time
        *        of the text cache destruction.
        */
        ~TextCache();

        /**
        * @brief Create and get glyph metrics for a string using a font instance
        *
        * Use this call to obtain glyph metadata - positions, sizes, language and font origin (contained in GlyphKey).
        * You can change the positions if you need to, e.g. if you need to do funky things like re-aligning glyphs
        * coming from different fonts with incompatible baselines. But in the regular case, you just pass the glyphs
        * to createTextLine() as-is.
        *
        * @param[in] str The string for which to create glyph metrics
        * @param[in] font Id of the font instance to be used for creating the glyph metrics vector.
        *                 The font instance must be available at the font accessor passed in the
        *                 constructor of the text cache.
        * @return The glyph metrics vector created
        */
        GlyphMetricsVector      getPositionedGlyphs(const std::u32string& str, FontInstanceId font);

        /**
        * @brief Create and get glyph metrics for a string using a list of font instances and offsets
        *
        * Use this version of getPositionedGlyphs if you need more fine-grained control over how glyphs are
        * resolved from multiple fonts. See also documentation of FontInstanceOffset.
        *
        * @param[in] str The string for which to create glyph metrics
        * @param[in] fontOffsets The font offsets created from font cascade to be used for creating the glyph metrics vector.
        *                 The font instances within the font cascade must all be available at the font accessor passed in the
        *                 constructor of the text cache. Also see docs of FontInstanceOffsets
        * @return The glyph metrics created
        */
        GlyphMetricsVector      getPositionedGlyphs(const std::u32string& str, const FontInstanceOffsets& fontOffsets);

        /**
        * @brief Create the scene objects, e.g., mesh and appearance...etc, needed for rendering a text line (represented by glyph metrics).
        * If the provided string of glyphs contains no render-able characters (e.g. it has only white spaces), the method will fail with an error.
        * If you want to avoid having such errors, filter out the \p glyphs with no visual
        * representation (e.g. control characters) and use the helper method ContainsRenderableGlyphs on top to check if the remaining glyphs
        * contain at least one renderable (size not zero) glyph so they can be used as input for createTextLine.
        *
        * This method will always produce exactly one MeshNode. We do this by enforcing that all glyphs are rendered in the same texture atlas page,
        * thus making it possible to create one mesh instead of several. The effect argument has special requirements - it needs to have
        * three semantic uniforms:
        * - EEffectAttributeSemantic::TextTexture - this is where TextCache will link the texture atlas page with glyph data
        * - EEffectAttributeSemantic::TextPositions - this is where the text quad vertices are linked
        * - EEffectAttributeSemantic::TextTextureCoordinates - this is where texture coordinates are linked
        *
        * @param[in] glyphs The glyph metrics for which to create a text line
        * @param[in] effect The effect used for creating the appearance of the text line and rendering the meshes
        * @return Id of the text line created
        */
        TextLineId              createTextLine(const GlyphMetricsVector& glyphs, const Effect& effect);

        /**
        * @brief Get a const pointer to a (previously created) text line object
        * @param[in] textId Id of the text line object to get
        * @return A pointer to the text line object, or nullptr on failure
        */
        TextLine const*         getTextLine(TextLineId textId) const;

        /**
        * @brief Get a (non-const) pointer to a (previously created) text line object
        * @param[in] textId Id of the text line object to get
        * @return A pointer to the text line object, or nullptr on failure
        */
        TextLine*               getTextLine(TextLineId textId);

        /**
        * @brief Delete an existing text line object
        * @param[in] textId Id of the text line object to delete
        * @return True on success, false otherwise
        */
        bool                    deleteTextLine(TextLineId textId);

        /**
        * @brief Check if provided GlyphMetricsVector contains at least one renderable glyph
        * If this functions returns false, the provided input cannot be used as input for the
        * function createTextLine. The function call will simply fail.
        * @param[in] glyphMetrics GlyphMetrics to be checked
        * @ return True if the provided vector contains at least one renderable glyph
        */
        static bool             ContainsRenderableGlyphs(const GlyphMetricsVector& glyphMetrics);

        /**
        * Stores internal data for implementation specifics of TextCache.
        */
        class TextCacheImpl* impl;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        TextCache(const TextCache& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        TextCache& operator=(const TextCache& other) = delete;
    };
}

#endif

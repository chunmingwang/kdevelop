/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "languagepreferences.h"

#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KTextEditor/CodeCompletionInterface>

#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>

#include "../completionsettings.h"
#include "../core.h"
#include "languageconfig.h"
#include "ui_languagepreferences.h"

using namespace KTextEditor;

namespace KDevelop
{

LanguagePreferences::LanguagePreferences(QWidget* parent)
    : ConfigPage(nullptr, LanguageConfig::self(), parent)
{
    preferencesDialog = new Ui::LanguagePreferences;
    preferencesDialog->setupUi(this);
    preferencesDialog->kcfg_minFilesForSimplifiedParsing->setSuffix(ki18ncp("@item:valuesuffix", " file", " files"));
}

void LanguagePreferences::notifySettingsChanged()
{
    auto& settings(static_cast<CompletionSettings&>(*ICore::self()->languageController()->completionSettings()));

    settings.emitChanged();
}

LanguagePreferences::~LanguagePreferences( )
{
    delete preferencesDialog;
}

void LanguagePreferences::apply()
{
    ConfigPage::apply();

    const auto documents = Core::self()->documentController()->openDocuments();
    for (KDevelop::IDocument* doc : documents) {
        if (Document* textDoc = doc->textDocument()) {
            const auto views = textDoc->views();
            for (View* view : views) {
                if (auto* cc = qobject_cast<CodeCompletionInterface*>(view)) {
                    cc->setAutomaticInvocationEnabled(preferencesDialog->kcfg_automaticInvocation->isChecked());
                }
            }
        }
    }

    notifySettingsChanged();
}

QString LanguagePreferences::name() const
{
    return i18n("Language Support");
}

QString LanguagePreferences::fullName() const
{
    return i18n("Configure Code-Completion and Semantic Highlighting");
}

QIcon LanguagePreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("page-zoom"));
}

}

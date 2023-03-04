/*
 * purple - Jabber Protocol Plugin
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#include <glib/gi18n-lib.h>

#include <purple.h>

#include "usermood.h"
#include "pep.h"
#include <string.h>

static PurpleMood moods[] = {
	{ .mood = "afraid", .description = N_("Afraid"), },
	{ .mood = "amazed", .description = N_("Amazed"), },
	{ .mood = "amorous", .description = N_("Amorous"), },
	{ .mood = "angry", .description = N_("Angry"), },
	{ .mood = "annoyed", .description = N_("Annoyed"), },
	{ .mood = "anxious", .description = N_("Anxious"), },
	{ .mood = "aroused", .description = N_("Aroused"), },
	{ .mood = "ashamed", .description = N_("Ashamed"), },
	{ .mood = "bored", .description = N_("Bored"), },
	{ .mood = "brave", .description = N_("Brave"), },
	{ .mood = "calm", .description = N_("Calm"), },
	{ .mood = "cautious", .description = N_("Cautious"), },
	{ .mood = "cold", .description = N_("Cold"), },
	{ .mood = "confident", .description = N_("Confident"), },
	{ .mood = "confused", .description = N_("Confused"), },
	{ .mood = "contemplative", .description = N_("Contemplative"), },
	{ .mood = "contented", .description = N_("Contented"), },
	{ .mood = "cranky", .description = N_("Cranky"), },
	{ .mood = "crazy", .description = N_("Crazy"), },
	{ .mood = "creative", .description = N_("Creative"), },
	{ .mood = "curious", .description = N_("Curious"), },
	{ .mood = "dejected", .description = N_("Dejected"), },
	{ .mood = "depressed", .description = N_("Depressed"), },
	{ .mood = "disappointed", .description = N_("Disappointed"), },
	{ .mood = "disgusted", .description = N_("Disgusted"), },
	{ .mood = "dismayed", .description = N_("Dismayed"), },
	{ .mood = "distracted", .description = N_("Distracted"), },
	{ .mood = "embarrassed", .description = N_("Embarrassed"), },
	{ .mood = "envious", .description = N_("Envious"), },
	{ .mood = "excited", .description = N_("Excited"), },
	{ .mood = "flirtatious", .description = N_("Flirtatious"), },
	{ .mood = "frustrated", .description = N_("Frustrated"), },
	{ .mood = "grateful", .description = N_("Grateful"), },
	{ .mood = "grieving", .description = N_("Grieving"), },
	{ .mood = "grumpy", .description = N_("Grumpy"), },
	{ .mood = "guilty", .description = N_("Guilty"), },
	{ .mood = "happy", .description = N_("Happy"), },
	{ .mood = "hopeful", .description = N_("Hopeful"), },
	{ .mood = "hot", .description = N_("Hot"), },
	{ .mood = "humbled", .description = N_("Humbled"), },
	{ .mood = "humiliated", .description = N_("Humiliated"), },
	{ .mood = "hungry", .description = N_("Hungry"), },
	{ .mood = "hurt", .description = N_("Hurt"), },
	{ .mood = "impressed", .description = N_("Impressed"), },
	{ .mood = "in_awe", .description = N_("In awe"), },
	{ .mood = "in_love", .description = N_("In love"), },
	{ .mood = "indignant", .description = N_("Indignant"), },
	{ .mood = "interested", .description = N_("Interested"), },
	{ .mood = "intoxicated", .description = N_("Intoxicated"), },
	{ .mood = "invincible", .description = N_("Invincible"), },
	{ .mood = "jealous", .description = N_("Jealous"), },
	{ .mood = "lonely", .description = N_("Lonely"), },
	{ .mood = "lost", .description = N_("Lost"), },
	{ .mood = "lucky", .description = N_("Lucky"), },
	{ .mood = "mean", .description = N_("Mean"), },
	{ .mood = "moody", .description = N_("Moody"), },
	{ .mood = "nervous", .description = N_("Nervous"), },
	{ .mood = "neutral", .description = N_("Neutral"), },
	{ .mood = "offended", .description = N_("Offended"), },
	{ .mood = "outraged", .description = N_("Outraged"), },
	{ .mood = "playful", .description = N_("Playful"), },
	{ .mood = "proud", .description = N_("Proud"), },
	{ .mood = "relaxed", .description = N_("Relaxed"), },
	{ .mood = "relieved", .description = N_("Relieved"), },
	{ .mood = "remorseful", .description = N_("Remorseful"), },
	{ .mood = "restless", .description = N_("Restless"), },
	{ .mood = "sad", .description = N_("Sad"), },
	{ .mood = "sarcastic", .description = N_("Sarcastic"), },
	{ .mood = "satisfied", .description = N_("Satisfied"), },
	{ .mood = "serious", .description = N_("Serious"), },
	{ .mood = "shocked", .description = N_("Shocked"), },
	{ .mood = "shy", .description = N_("Shy"), },
	{ .mood = "sick", .description = N_("Sick"), },
	{ .mood = "sleepy", .description = N_("Sleepy"), },
	{ .mood = "spontaneous", .description = N_("Spontaneous"), },
	{ .mood = "stressed", .description = N_("Stressed"), },
	{ .mood = "strong", .description = N_("Strong"), },
	{ .mood = "surprised", .description = N_("Surprised"), },
	{ .mood = "thankful", .description = N_("Thankful"), },
	{ .mood = "thirsty", .description = N_("Thirsty"), },
	{ .mood = "tired", .description = N_("Tired"), },
	{ .mood = "undefined", .description = N_("Undefined"), },
	{ .mood = "weak", .description = N_("Weak"), },
	{ .mood = "worried", .description = N_("Worried"), },
	/* Mark last record. */
	{ .mood = NULL, .description = NULL, }
};

static const PurpleMood*
find_mood_by_name(const gchar *name)
{
	int i;

	g_return_val_if_fail(name && *name, NULL);

	for (i = 0; moods[i].mood != NULL; ++i) {
		if (purple_strequal(name, moods[i].mood)) {
			return &moods[i];
		}
	}

	return NULL;
}

static void jabber_mood_cb(JabberStream *js, const char *from, PurpleXmlNode *items) {
	/* it doesn't make sense to have more than one item here, so let's just pick the first one */
	PurpleXmlNode *item = purple_xmlnode_get_child(items, "item");
	const char *newmood = NULL;
	char *moodtext = NULL;
	JabberBuddy *buddy = jabber_buddy_find(js, from, FALSE);
	PurpleXmlNode *moodinfo, *mood;
	/* ignore the mood of people not on our buddy list */
	if (!buddy || !item)
		return;

	mood = purple_xmlnode_get_child_with_namespace(item, "mood", "http://jabber.org/protocol/mood");
	if (!mood)
		return;
	for (moodinfo = mood->child; moodinfo; moodinfo = moodinfo->next) {
		if (moodinfo->type == PURPLE_XMLNODE_TYPE_TAG) {
			if (purple_strequal(moodinfo->name, "text")) {
				if (!moodtext) /* only pick the first one */
					moodtext = purple_xmlnode_get_data(moodinfo);
			} else {
				const PurpleMood *target_mood;

				/* verify that the mood is known (valid) */
				target_mood = find_mood_by_name(moodinfo->name);
				newmood = target_mood ? target_mood->mood : NULL;
			}

		}
		if (newmood != NULL && moodtext != NULL)
			break;
	}
	if (newmood != NULL) {
		purple_protocol_got_user_status(purple_connection_get_account(js->gc), from, "mood",
				PURPLE_MOOD_NAME, newmood,
				PURPLE_MOOD_COMMENT, moodtext,
				NULL);
	} else {
		purple_protocol_got_user_status_deactive(purple_connection_get_account(js->gc), from, "mood");
	}
	g_free(moodtext);
}

void jabber_mood_init(void) {
	jabber_add_feature("http://jabber.org/protocol/mood", jabber_pep_namespace_only_when_pep_enabled_cb);
	jabber_pep_register_handler("http://jabber.org/protocol/mood", jabber_mood_cb);
}

gboolean
jabber_mood_set(JabberStream *js, const char *mood, const char *text)
{
	const PurpleMood *target_mood = NULL;
	PurpleXmlNode *publish, *moodnode;

	if (mood && *mood) {
		target_mood = find_mood_by_name(mood);
		/* Mood specified, but is invalid --
		 * fail so that the command can handle this.
		 */
		if (!target_mood)
			return FALSE;
	}

	publish = purple_xmlnode_new("publish");
	purple_xmlnode_set_attrib(publish,"node","http://jabber.org/protocol/mood");
	moodnode = purple_xmlnode_new_child(purple_xmlnode_new_child(publish, "item"), "mood");
	purple_xmlnode_set_namespace(moodnode, "http://jabber.org/protocol/mood");

	if (target_mood) {
		/* If target_mood is not NULL, then
		 * target_mood->mood == mood, and is a valid element name.
		 */
	    purple_xmlnode_new_child(moodnode, mood);

		/* Only set text when setting a mood */
		if (text && *text) {
			PurpleXmlNode *textnode = purple_xmlnode_new_child(moodnode, "text");
			purple_xmlnode_insert_data(textnode, text, -1);
		}
	}

	jabber_pep_publish(js, publish);
	return TRUE;
}

PurpleMood *
jabber_get_moods(G_GNUC_UNUSED PurpleProtocolClient *client,
                 G_GNUC_UNUSED PurpleAccount *account)
{
	return moods;
}

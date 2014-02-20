import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.socialcache 1.0
import "shared"

SocialMediaFeedPage {
    id: page
    configKey: "/sailfish/events_view/facebook"
    timestampRole: 3 //FacebookPostsModel.Timestamp
    listModel: FacebookPostsModel {
        id: facebookPostsModel
    }

    //: Facebook service name
    //% "Facebook"
    headerTitle: qsTrId("lipstick-jolla-home-la-facebook")
    listDelegate: FacebookFeedItem {
        id: feedItem
        connectedToNetwork: page.connectedToNetwork
        width: page.width
        imageList: model.images
        onClicked: {
            var rv = null
            try {
                rv = pageStack.push(Qt.resolvedUrl("FacebookPostPage.qml"), {
                                        "model": model, "subviewModel": subviewModel,
                                        "connectedToNetwork": Qt.binding(function() { return page.connectedToNetwork })
                                    }, false)
            } catch (error) {
                console.log(error)
            }
        }
        Component.onCompleted: feedItem.refreshTimeCount = Qt.binding(function() { return page.refreshTimeCount })
    }
    socialNetwork: SocialSync.Facebook
    syncNotifications: true

    PullDownMenu {
        flickable: page.listView
        busy: page.updating

        MenuItem {
            //: Opens Facebook in browser
            //% "Go to Facebook"
            text: qsTrId("lipstick-jolla-home-me-go_to_facebook")
            onClicked: Qt.openUrlExternally("https://m.facebook.com")
        }

        MenuItem {
            //: Update social media feed
            //% "Update"
            text: qsTrId("lipstick-jolla-home-me-update_feed")
            onClicked: page.sync()
        }
    }
}
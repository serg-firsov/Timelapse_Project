/**
 * Created by Sergey Firsov on 25.08.2016.
 */

$(document).ready(function () {
    if(window.innerWidth < 900) {
        $(".btn-responsive").hide();
    }
    else {
        $(".btn-responsive").show();
    }
});

function onClickWelcome() {
    $('html, body').animate({scrollTop: 0});
}

function onClickAssemblingHardware() {
    var elementPosition = $('#assembling-hardware').position().top;
    $('html, body').animate({scrollTop: elementPosition});
}

function onClickInstallingSoftware() {
    var elementPosition = $('#installing-software').position().top;
    $('html, body').animate({scrollTop: elementPosition});
}

function onClickControlsOverview() {
    var elementPosition = $('#controls-overview').position().top;
    $('html, body').animate({scrollTop: elementPosition});
}

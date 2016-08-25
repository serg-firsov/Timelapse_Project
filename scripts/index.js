/**
 * Created by Sergey Firsov on 25.08.2016.
 */

function onClickWelcome() {
    $('body').animate({scrollTop: 0});
}

function onClickAssemblingHardware() {
    var elementPosition = $('#assembling-hardware').position().top;
    $('body').animate({scrollTop: elementPosition});
}

function onClickInstallingSoftware() {
    var elementPosition = $('#installing-software').position().top;
    $('body').animate({scrollTop: elementPosition});
}

function onClickControlsOverview() {
    var elementPosition = $('#controls-overview').position().top;
    $('body').animate({scrollTop: elementPosition});
}

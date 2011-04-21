$(document).ready(function() {
    $("a.example-image-link").click(function() {
        $("#hotseat").attr("src",$(this).attr("href"));
        return false;
    });
});
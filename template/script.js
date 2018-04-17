$(document).ready ->
  is_dragging = false
  
  $(document).on "mousedown touchstart", ".circle", (e) ->
    is_dragging = true
  
  $(document).on "mouseup touchend", (e) ->
    is_dragging = false
    
  $(window).on "mousemove touchmove", (e) ->
    if is_dragging
      circle = $(".circle")
      touch = undefined
      if e.originalEvent.touches
        touch = e.originalEvent.touches[0]
      center_x = ($(circle).outerWidth() / 2) + $(circle).offset().left
      center_y = ($(circle).outerHeight() / 2) + $(circle).offset().top
      pos_x = e.pageX or touch.pageX
      pos_y = e.pageY or touch.pageY
      delta_y =  center_y - pos_y
      delta_x = center_x - pos_x
      angle = Math.atan2(delta_y, delta_x) * (180 / Math.PI) # Calculate Angle between circle center and mouse pos
      angle -= 90
      if angle < 0
        angle = 360 + angle # Always show angle positive
      angle = Math.round(angle)
      $(".dot").css("transform", "rotate("+angle+"deg)")
      $(".debug").html(angle + "deg")
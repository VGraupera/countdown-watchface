var app = angular.module('eventsApp', ['ngRoute']);

app.factory('Events', function () {
  var events = [];
  return {
    data: function () {

      if (localStorage.getItem("eventList") === null) {
        events = [{
          name: 'Christmas',
          date: '2014-12-25'
        }];
      } else {
        events = JSON.parse(localStorage.getItem("eventList")); // get value from localstorage
      }

      return events;
    },
    save: function () {
      localStorage.setItem("eventList", JSON.stringify(events)); // saving array in html5 local storage
    }
  }
});

app.config(function($routeProvider) {
  $routeProvider
  .when('/', {
    controller:'ListCtrl',
    templateUrl:'list.html'
  })
  .when('/edit/:index', {
    controller:'EditCtrl',
    templateUrl:'detail.html'
  })
  .when('/new', {
    controller:'CreateCtrl',
    templateUrl:'detail.html'
  })
  .otherwise({
    redirectTo:'/'
  });
});

app.controller('ListCtrl', function($scope, Events){
  $scope.events = Events.data();

  $scope.save = function () {
    document.location = 'pebblejs://close#' + encodeURIComponent(JSON.stringify( $scope.events ));
  };
});

app.controller('EditCtrl', function($scope, $location, Events, $routeParams){
  $scope.events = Events.data();
  $scope.event = $scope.events[$routeParams.index];

  $scope.destroy = function(item) {
    var index=$scope.events.indexOf(item);
    $scope.events.splice(index,1);
    Events.save();
    $location.path('/');
  };

  $scope.save = function() {
    Events.save();
    $location.path('/');
  };
});

app.controller('CreateCtrl', function($scope, $location, Events, $routeParams){
  $scope.events = Events.data();
  $scope.save = function() {
    $scope.events.push($scope.event);
    Events.save();
    $location.path('/');
  };
});
